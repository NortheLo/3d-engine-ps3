#include <iostream>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <ppu-types.h>
#include <math.h>
#include <sys/process.h>

#include <io/pad.h>
#include <rsx/rsx.h>
#include <sysutil/sysutil.h>

#include "acid.h"
#include "mesh.h"
#include "rsxutil.h"

#include "diffuse_specular_shader_vpo.h"
#include "diffuse_specular_shader_fpo.h"

#define M_PI 3.14159265358979323846

#define DEGTORAD(a)			( (a) * 0.01745329252f )
#define RADTODEG(a)			( (a) * 57.29577951f )

u32 running = 0;

u32 fp_offset;
u32 *fp_buffer;

u32 *texture_buffer;
u32 texture_offset;

// vertex shader
rsxProgramConst *projMatrix;
rsxProgramConst *mvMatrix;

// fragment shader
rsxProgramAttrib *textureUnit;
rsxProgramConst *eyePosition;
rsxProgramConst *globalAmbient;
rsxProgramConst *litPosition;
rsxProgramConst *litColor;
rsxProgramConst *Kd;
rsxProgramConst *Ks;
rsxProgramConst *spec;

/* position vectors */
Vector3 fwd 	= Vector3(0.f, 0.f, -1.f);
Vector3 right 	= Vector3(1.f, 0.f, 0.f);
Vector3 up 		= Vector3(0.f, 1.f, 0.f);

typedef struct {
	Vector3 pos = Vector3(0.f, 0.f, 4.f);
	float yaw = -90.f;
	float pitch = 0.f;
} camera;

void *vp_ucode = NULL;
rsxVertexProgram *vpo =(rsxVertexProgram*)diffuse_specular_shader_vpo;

void *fp_ucode = NULL;
rsxFragmentProgram *fpo =(rsxFragmentProgram*)diffuse_specular_shader_fpo;

static Matrix4 projectionMatrix;
static SMeshBuffer *cube = NULL;

SYS_PROCESS_PARAM(1001, 0x100000);

extern "C" {
static void program_exit_callback()
{
	gcmSetWaitFlip(context);
	rsxFinish(context,1);
}

static void sysutil_exit_callback(u64 status,u64 param,void *usrdata)
{
	switch(status) {
		case SYSUTIL_EXIT_GAME:
			running = 0;
			break;
		case SYSUTIL_DRAW_BEGIN:
		case SYSUTIL_DRAW_END:
			break;
		default:
			break;
	}
}
}
static void init_texture()
{
	u32 i;
	u8 *buffer;
	const u8 *data = acid.pixel_data;

	texture_buffer = (u32*)rsxMemalign(128,(acid.width*acid.height*4));
	if(!texture_buffer) return;

	rsxAddressToOffset(texture_buffer,&texture_offset);

	buffer = (u8*)texture_buffer;
	for(i=0;i<acid.width*acid.height*4;i+=4) {
		buffer[i + 1] = *data++;
		buffer[i + 2] = *data++;
		buffer[i + 3] = *data++;
		buffer[i + 0] = *data++;
	}
}

/*
	Convert float input from degree to radians.
*/
float deg2rad(float deg) {
	return deg * (M_PI / 180);
}

/*
	Clamp value to given floors.
*/
float clamp(float n, float lower, float upper) {
  return std::max(lower, std::min(n, upper));
}

static SMeshBuffer* createCube(f32 size)
{
	u32 i;
	SMeshBuffer *buffer = new SMeshBuffer();
	const u16 u[36] = {   0,1,2,   0,2,3,   1,4,5,   1,5,2,   4,7,6,	 4,6,5, 
						  7,0,3,   7,3,6,   9,2,5,   9,5,8,   0,10,11,   0,7,10};

	buffer->cnt_indices = 36;
	buffer->indices = (u16*)rsxMemalign(128,buffer->cnt_indices*sizeof(u16));

	for(i=0;i<36;i++) buffer->indices[i] = u[i];

	buffer->cnt_vertices = 12;
	buffer->vertices = (S3DVertex*)rsxMemalign(128,buffer->cnt_vertices*sizeof(S3DVertex));

	buffer->vertices[0] = S3DVertex(0,0,0, -1,-1,-1, 1, 0);
	buffer->vertices[1] = S3DVertex(1,0,0,  1,-1,-1, 1, 1);
	buffer->vertices[2] = S3DVertex(1,1,0,  1, 1,-1, 0, 1);
	buffer->vertices[3] = S3DVertex(0,1,0, -1, 1,-1, 0, 0);
	buffer->vertices[4] = S3DVertex(1,0,1,  1,-1, 1, 1, 0);
	buffer->vertices[5] = S3DVertex(1,1,1,  1, 1, 1, 0, 0);
	buffer->vertices[6] = S3DVertex(0,1,1, -1, 1, 1, 0, 1);
	buffer->vertices[7] = S3DVertex(0,0,1, -1,-1, 1, 1, 1);
	buffer->vertices[8] = S3DVertex(0,1,1, -1, 1, 1, 1, 0);
	buffer->vertices[9] = S3DVertex(0,1,0, -1, 1,-1, 1, 1);
	buffer->vertices[10] = S3DVertex(1,0,1,  1,-1, 1, 0, 1);
	buffer->vertices[11] = S3DVertex(1,0,0,  1,-1,-1, 0, 0);

	for(i=0;i<12;i++) {
		buffer->vertices[i].pos -= Vector3(0.5f,0.5f,0.5f);
		buffer->vertices[i].pos *= size;
	}

	return buffer;
}

static void setTexture(u8 textureUnit) {
	u32 width = 128;
	u32 height = 128;
	u32 pitch = (width*4);
	gcmTexture texture;

	if(!texture_buffer) return;

	rsxInvalidateTextureCache(context,GCM_INVALIDATE_TEXTURE);

	texture.format		= (GCM_TEXTURE_FORMAT_A8R8G8B8 | GCM_TEXTURE_FORMAT_LIN);
	texture.mipmap		= 1;
	texture.dimension	= GCM_TEXTURE_DIMS_2D;
	texture.cubemap		= GCM_FALSE;
	texture.remap		= ((GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_B_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_G_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_R_SHIFT) |
						   (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_A_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_B << GCM_TEXTURE_REMAP_COLOR_B_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_G << GCM_TEXTURE_REMAP_COLOR_G_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_R << GCM_TEXTURE_REMAP_COLOR_R_SHIFT) |
						   (GCM_TEXTURE_REMAP_COLOR_A << GCM_TEXTURE_REMAP_COLOR_A_SHIFT));
	texture.width		= width;
	texture.height		= height;
	texture.depth		= 1;
	texture.location	= GCM_LOCATION_RSX;
	texture.pitch		= pitch;
	texture.offset		= texture_offset;
	rsxLoadTexture(context,textureUnit,&texture);
	rsxTextureControl(context,textureUnit,GCM_TRUE,0<<8,12<<8,GCM_TEXTURE_MAX_ANISO_1);
	rsxTextureFilter(context,textureUnit,0,GCM_TEXTURE_LINEAR,GCM_TEXTURE_LINEAR,GCM_TEXTURE_CONVOLUTION_QUINCUNX);
	rsxTextureWrapMode(context,textureUnit,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,GCM_TEXTURE_CLAMP_TO_EDGE,0,GCM_TEXTURE_ZFUNC_LESS,0);
}

static void setDrawEnv()
{
	rsxSetColorMask(context,GCM_COLOR_MASK_B |
							GCM_COLOR_MASK_G |
							GCM_COLOR_MASK_R |
							GCM_COLOR_MASK_A);

	rsxSetColorMaskMrt(context,0);

	u16 x,y,w,h;
	f32 min, max;
	f32 scale[4],offset[4];

	x = 0;
	y = 0;
	w = display_width;
	h = display_height;
	min = 0.0f;
	max = 1.0f;
	scale[0] = w*0.5f;
	scale[1] = h*-0.5f;
	scale[2] = (max - min)*0.5f;
	scale[3] = 0.0f;
	offset[0] = x + w*0.5f;
	offset[1] = y + h*0.5f;
	offset[2] = (max + min)*0.5f;
	offset[3] = 0.0f;

	rsxSetViewport(context,x, y, w, h, min, max, scale, offset);
	rsxSetScissor(context,x,y,w,h);

	rsxSetDepthTestEnable(context,GCM_TRUE);
	rsxSetDepthFunc(context,GCM_LESS);
	rsxSetShadeModel(context,GCM_SHADE_MODEL_SMOOTH);
	rsxSetDepthWriteEnable(context,1);
	rsxSetFrontFace(context,GCM_FRONTFACE_CCW);
}

void init_shader()
{
	u32 fpsize = 0;
	u32 vpsize = 0;

	rsxVertexProgramGetUCode(vpo, &vp_ucode, &vpsize);
	printf("vpsize: %d\n", vpsize);

	projMatrix = rsxVertexProgramGetConst(vpo,"projMatrix");
	mvMatrix = rsxVertexProgramGetConst(vpo,"modelViewMatrix");

	rsxFragmentProgramGetUCode(fpo, &fp_ucode, &fpsize);
	printf("fpsize: %d\n", fpsize);

	fp_buffer = (u32*)rsxMemalign(64,fpsize);
	memcpy(fp_buffer,fp_ucode,fpsize);
	rsxAddressToOffset(fp_buffer,&fp_offset);

	textureUnit = rsxFragmentProgramGetAttrib(fpo,"texture");
	eyePosition = rsxFragmentProgramGetConst(fpo,"eyePosition");
	globalAmbient = rsxFragmentProgramGetConst(fpo,"globalAmbient");
	litPosition = rsxFragmentProgramGetConst(fpo,"lightPosition");
	litColor = rsxFragmentProgramGetConst(fpo,"lightColor");
	spec = rsxFragmentProgramGetConst(fpo,"shininess");
	Ks = rsxFragmentProgramGetConst(fpo,"Ks");
	Kd = rsxFragmentProgramGetConst(fpo,"Kd");
}

void drawFrame(camera* camera) {
	u32 offset, color = 0;
	Matrix4 rotX,rotY;
	Vector4 objEyePos, objLightPos;
	Matrix4 modelMatrix, modelMatrixIT, modelViewMatrix;
	Point3 lightPos = Point3(250.0f, 150.0f, 150.0f);
	f32 globalAmbientColor[3] = {0.1f, 0.1f, 0.1f};
	f32 lightColor[3] = {0.95f, 0.95f, 0.95f};
	f32 materialColorDiffuse[3] = {0.5f, 0.0f, 0.0f};
	f32 materialColorSpecular[3] = {0.7f, 0.6f, 0.6f};
	f32 shininess = 17.8954f;
	SMeshBuffer *mesh = cube;

	Matrix4 viewMatrix = Matrix4::lookAt((Point3) camera->pos, (Point3) (camera->pos + fwd), up);


	setDrawEnv();
	setTexture(textureUnit->index);

	rsxSetClearColor(context, color);
	rsxSetClearDepthStencil(context, 0xffffff00);
	rsxClearSurface(context, GCM_CLEAR_R | GCM_CLEAR_G | GCM_CLEAR_B |
							 GCM_CLEAR_A | GCM_CLEAR_S | GCM_CLEAR_Z);

	rsxSetZMinMaxControl(context, 0, 1, 1);

	for(size_t i = 0; i < 8; i++)
		rsxSetViewportClip(context, i, display_width, display_height);

	modelMatrixIT = inverse(modelMatrix);
	modelViewMatrix = transpose(viewMatrix);
	objEyePos = modelMatrixIT * (Point3) camera->pos;
	objLightPos = modelMatrixIT * lightPos;

	rsxAddressToOffset(&mesh->vertices[0].pos, &offset);
	rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_POS, 0, offset, sizeof(S3DVertex), 3, GCM_VERTEX_DATA_TYPE_F32, GCM_LOCATION_RSX);

	// load texture
	rsxAddressToOffset(&mesh->vertices[0].nrm, &offset);
	rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_NORMAL, 0, offset, sizeof(S3DVertex), 3, GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);

	rsxAddressToOffset(&mesh->vertices[0].u, &offset);
	rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_TEX0, 0, offset, sizeof(S3DVertex), 2, GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);

	rsxLoadVertexProgram(context, vpo,vp_ucode);
	rsxSetVertexProgramParameter(context, vpo, projMatrix, (float*) &projectionMatrix);
	rsxSetVertexProgramParameter(context, vpo, mvMatrix, (float*) &modelViewMatrix);

	rsxSetFragmentProgramParameter(context, fpo, eyePosition, (float*) &objEyePos, fp_offset, GCM_LOCATION_RSX);
	rsxSetFragmentProgramParameter(context, fpo, globalAmbient, globalAmbientColor, fp_offset, GCM_LOCATION_RSX);
	rsxSetFragmentProgramParameter(context, fpo, litPosition, (float*) &objLightPos, fp_offset, GCM_LOCATION_RSX);
	rsxSetFragmentProgramParameter(context, fpo, litColor, lightColor, fp_offset, GCM_LOCATION_RSX);
	rsxSetFragmentProgramParameter(context, fpo, spec, &shininess, fp_offset, GCM_LOCATION_RSX);

	rsxSetFragmentProgramParameter(context, fpo, Kd, materialColorDiffuse, fp_offset, GCM_LOCATION_RSX);
	rsxSetFragmentProgramParameter(context, fpo, Ks, materialColorSpecular, fp_offset, GCM_LOCATION_RSX);

	rsxLoadFragmentProgramLocation(context, fpo, fp_offset, GCM_LOCATION_RSX);

	rsxSetUserClipPlaneControl(context,GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE,
									   GCM_USER_CLIP_PLANE_DISABLE);

	rsxAddressToOffset(&mesh->indices[0], &offset);
	rsxDrawIndexArray(context, GCM_TYPE_TRIANGLES, offset, mesh->cnt_indices, GCM_INDEX_TYPE_16B, GCM_LOCATION_RSX);
}

/*
	Calculate normalize the position from the sticks, so the center/default position is 0.f.
	The value ranges from 1.f to -1.f depending from the position to the default position.
*/
float normalizeAnalogSticks(float raw) {
	float normalized = (raw - 128.f) / 128.f;
	return normalized;
}

/*
	Calculate the new camera position from the stick inputs.
*/
void getCameraPosition(camera* cam, padData* pad) {
	cam->pos   -= 0.1 * fwd * 	normalizeAnalogSticks((f32) pad->ANA_L_V);
	cam->pos   += 0.1 * right * normalizeAnalogSticks((f32) pad->ANA_L_H);
	cam->yaw   += 				normalizeAnalogSticks((f32) pad->ANA_R_H);
	cam->pitch += 				normalizeAnalogSticks((f32) pad->ANA_R_V);
	cam->pitch  = clamp(cam->pitch, -89.f, 89.f);
}

/* 
	Calculate the new forward, right and up vector from the new camera position.
*/
void updateCamera(camera* cam) {
	float yaw = deg2rad(cam->yaw);
	float pitch = deg2rad(cam->pitch);

	Vector3 new_fwd = {cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)};
	new_fwd = normalize(new_fwd);
	fwd = new_fwd;

	Vector3 new_right = normalize(cross(fwd, (Vector3) {0.f, 1.f, 0.f}));
	right = new_right;

	Vector3 new_up = normalize(cross(right, fwd));
	up = new_up;
}

int main() {
	camera camera;
	padInfo padinfo;
	padData paddata;

	void *host_addr = memalign(HOST_ADDR_ALIGNMENT, HOSTBUFFER_SIZE);

	init_screen(host_addr, HOSTBUFFER_SIZE);
	ioPadInit(1);
	init_shader();
	init_texture();

	cube = createCube(5.0f);

	atexit(program_exit_callback);
	sysUtilRegisterCallback(0, sysutil_exit_callback, NULL);

	setDrawEnv();
	setRenderTarget(curr_fb);

	running = 1;
	while(running) {
		sysUtilCheckCallback();

		ioPadGetInfo(&padinfo);

		if(padinfo.status[0]) {
			ioPadGetData(0, &paddata);

			getCameraPosition(&camera, &paddata);
			updateCamera(&camera);

			if(paddata.BTN_START)
				goto done;
		}
		
		
		drawFrame(&camera);
		projectionMatrix = transpose(Matrix4::perspective(DEGTORAD(90.f), aspect_ratio, 1.0f, 3000.0f));

		flip();
	}

	done:
		program_exit_callback();
		return 0;
}