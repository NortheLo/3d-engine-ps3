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

Point3 eye_pos = Point3(0.0f, 0.0f, 20.0f);
Point3 eye_dir = Point3(0.0f, 0.0f, 0.0f);
Vector3 up_vec = Vector3(0.0f, 1.0f, 0.0f);

typedef struct {
	f32 pos_x, pos_z, angle_y, angle_x;
} cam;

void *vp_ucode = NULL;
rsxVertexProgram *vpo =(rsxVertexProgram*)diffuse_specular_shader_vpo;

void *fp_ucode = NULL;
rsxFragmentProgram *fpo =(rsxFragmentProgram*)diffuse_specular_shader_fpo;

static Matrix4 P;
static SMeshBuffer *sphere = NULL;
static SMeshBuffer *donut = NULL;
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

static SMeshBuffer* createDonut(f32 outerRadius,f32 innerRadius,u32 polyCntX,u32 polyCntY)
{
	u32 i,x,y,level;
	SMeshBuffer *buffer = new SMeshBuffer();

	if(polyCntX<2) polyCntX = 2;
	if(polyCntY<2) polyCntY = 2;
	while(polyCntX*polyCntY>32767) {
		polyCntX /= 2;
		polyCntY /= 2;
	}

	f32 ay = 0;
	const f32 angleX = 2*M_PI/polyCntX;
	const f32 angleY = 2*M_PI/polyCntY;
	const u32 polyCntXpitch = polyCntX +1;
	const u32 polyCntYpitch = polyCntY + 1;

	buffer->cnt_vertices = polyCntYpitch*polyCntXpitch;
	buffer->vertices = (S3DVertex*)rsxMemalign(128,buffer->cnt_vertices*sizeof(S3DVertex));

	buffer->cnt_indices = polyCntY*polyCntX*6;
	buffer->indices = (u16*)rsxMemalign(128,buffer->cnt_indices*sizeof(u16));

	i = 0;
	for(y=0;y<=polyCntY;y++) {
		f32 axz = 0;

		const f32 sinay = sinf(ay);
		const f32 cosay = cosf(ay);
		const f32 tu = (f32)y/(f32)polyCntY;
		for(x=0;x<=polyCntX;x++) {
			const Vector3 pos(static_cast<f32>((outerRadius - (innerRadius*cosf(axz)))*cosay),
									  static_cast<f32>((outerRadius - (innerRadius*cosf(axz)))*sinay),
									  static_cast<f32>(innerRadius*sinf(axz)));
			
			const Vector3 nrm(static_cast<f32>(-cosf(axz)*cosay),
									  static_cast<f32>(-cosf(axz)*sinay),
									  static_cast<f32>(sinf(axz)));

			buffer->vertices[i] = S3DVertex(pos.getX(),pos.getY(),pos.getZ(),nrm.getX(),nrm.getY(),nrm.getZ(),tu,(f32)x/(f32)polyCntX);

			axz += angleX;
			i++;
		}
		ay += angleY;
	}

	i = 0;
	level = 0;
	for(y=0;y<polyCntY;y++) {
		for(x=0;x<polyCntX - 1;x++) {
			const u32 curr = level + x;
			buffer->indices[i++] = curr;
			buffer->indices[i++] = curr + polyCntXpitch;
			buffer->indices[i++] = curr + 1 + polyCntXpitch;
			
			buffer->indices[i++] = curr;
			buffer->indices[i++] = curr + 1 + polyCntXpitch;
			buffer->indices[i++] = curr + 1;
		}

		buffer->indices[i++] = level + polyCntX;
		buffer->indices[i++] = level + polyCntX - 1;
		buffer->indices[i++] = level + polyCntX - 1 + polyCntXpitch;
		
		buffer->indices[i++] = level + polyCntX;
		buffer->indices[i++] = level + polyCntX - 1 + polyCntXpitch;
		buffer->indices[i++] = level + polyCntX + polyCntXpitch;

		level += polyCntXpitch;
	}

	return buffer;
}

static SMeshBuffer* createSphere(f32 radius,u32 polyCntX,u32 polyCntY)
{
	u32 i,p1,p2,level;
	u32 x,y,polyCntXpitch;
	const f32 RECIPROCAL_PI = 1.0f/M_PI;
	SMeshBuffer *buffer = new SMeshBuffer();

	if(polyCntX<2) polyCntX = 2;
	if(polyCntY<2) polyCntY = 2;
	if(polyCntX*polyCntY>32767) {
		if(polyCntX>polyCntY) 
			polyCntX = 32767/polyCntY-1;
		else
			polyCntY = 32767/(polyCntX+1);
	}
	polyCntXpitch = polyCntX+1;

	buffer->cnt_vertices = (polyCntXpitch*polyCntY)+2;
	buffer->vertices = (S3DVertex*)rsxMemalign(128,buffer->cnt_vertices*sizeof(S3DVertex));

	buffer->cnt_indices = (polyCntX*polyCntY)*6;
	buffer->indices = (u16*)rsxMemalign(128,buffer->cnt_indices*sizeof(u16));

	i = 0;
	level = 0;
	for(p1=0;p1<polyCntY-1;p1++) {
		for(p2=0;p2<polyCntX-1;p2++) {
			const u32 curr = level + p2;
			buffer->indices[i++] = curr;
			buffer->indices[i++] = curr + polyCntXpitch;
			buffer->indices[i++] = curr + 1 + polyCntXpitch;

			buffer->indices[i++] = curr;
			buffer->indices[i++] = curr + 1 + polyCntXpitch;
			buffer->indices[i++] = curr + 1;
		}

		buffer->indices[i++] = level + polyCntX;
		buffer->indices[i++] = level + polyCntX - 1;
		buffer->indices[i++] = level + polyCntX - 1 + polyCntXpitch;

		buffer->indices[i++] = level + polyCntX;
		buffer->indices[i++] = level + polyCntX - 1 + polyCntXpitch;
		buffer->indices[i++] = level + polyCntX + polyCntXpitch;

		level += polyCntXpitch;
	}

	const u32 polyCntSq = polyCntXpitch*polyCntY;
	const u32 polyCntSq1 = polyCntSq+1;
	const u32 polyCntSqM1 = (polyCntY-1)*polyCntXpitch;

	for(p2=0;p2<polyCntX-1;p2++) {
		buffer->indices[i++] = polyCntSq;
		buffer->indices[i++] = p2;
		buffer->indices[i++] = p2+1;

		buffer->indices[i++] = polyCntSq1;
		buffer->indices[i++] = polyCntSqM1+p2;
		buffer->indices[i++] = polyCntSqM1+p2+1;
	}

	buffer->indices[i++] = polyCntSq;
	buffer->indices[i++] = polyCntX-1;
	buffer->indices[i++] = polyCntX;

	buffer->indices[i++] = polyCntSq1;
	buffer->indices[i++] = polyCntSqM1;
	buffer->indices[i++] = polyCntSqM1+polyCntX-1;

	f32 axz;
	f32 ay = 0;
	const f32 angelX = 2*M_PI/polyCntX;
	const f32 angelY = M_PI/polyCntY;

	i = 0;
	for(y=0;y<polyCntY;y++) {
		axz = 0;
		ay += angelY;
		const f32 sinay = sinf(ay);
		for(x=0;x<polyCntX;x++) {
			const Vector3 pos(static_cast<f32>(radius*cosf(axz)*sinay), static_cast<f32>(radius*cosf(ay)), static_cast<f32>(radius*sinf(axz)*sinay));
			
			Vector3 normal = normalize(pos);
			
			f32 tu = 0.5F;
			if(y==0) {
				if(normal.getY()!=-1.0F && normal.getY()!=1.0F)
					tu = static_cast<f32>(acosf(clamp(normal.getX()/sinay,-1.0f,1.0f))*0.5F*RECIPROCAL_PI);
				if(normal.getZ()<0.0F)
					tu = 1-tu;
			} else
				tu = buffer->vertices[i - polyCntXpitch].u;

			buffer->vertices[i] = S3DVertex(pos.getX(),pos.getY(),pos.getZ(),normal.getX(),normal.getY(),normal.getZ(),tu,static_cast<f32>(ay*RECIPROCAL_PI));
			axz += angelX;
			i++;
		}
		buffer->vertices[i] = S3DVertex(buffer->vertices[i-polyCntX]);
		buffer->vertices[i].u = 1.0F;
		i++;
	}

	buffer->vertices[i++] = S3DVertex(0.0F,radius,0.0F,0.0F,1.0F,0.0F,0.5F,0.0F);
	buffer->vertices[i] = S3DVertex(0.0F,-radius,0.0F,0.0F,-1.0F,0.0F,0.5F,1.0F);

	return buffer;
}

static void setTexture(u8 textureUnit)
{
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

float normalizeAnalogSticks(float raw) {
	float normalized = (raw - 128.f) / 128.f;
	return normalized;
}

void drawFrame(cam* camDat) {
	u32 offset, color = 0;
	Matrix4 rotX,rotY;
	Vector4 objEyePos, objLightPos;
	Matrix4 viewMatrix, modelMatrix, modelMatrixIT, modelViewMatrix;
	Point3 lightPos = Point3(250.0f, 150.0f, 150.0f);
	f32 globalAmbientColor[3] = {0.1f, 0.1f, 0.1f};
	f32 lightColor[3] = {0.95f, 0.95f, 0.95f};
	f32 materialColorDiffuse[3] = {0.5f, 0.0f, 0.0f};
	f32 materialColorSpecular[3] = {0.7f, 0.6f, 0.6f};
	f32 shininess = 17.8954f;
	SMeshBuffer *mesh = cube;

	setDrawEnv();
	setTexture(textureUnit->index);

	rsxSetClearColor(context, color);
	rsxSetClearDepthStencil(context, 0xffffff00);
	rsxClearSurface(context, GCM_CLEAR_R | GCM_CLEAR_G | GCM_CLEAR_B |
							 GCM_CLEAR_A | GCM_CLEAR_S | GCM_CLEAR_Z);

	rsxSetZMinMaxControl(context, 0, 1, 1);

	for(size_t i = 0; i < 8; i++)
		rsxSetViewportClip(context, i, display_width, display_height);


	// jdh approach
	Vector3 direction = Vector3(camDat->pos_x, 0.f, camDat->pos_z);
	Vector3 forward = Vector3(sinf(camDat->angle_y), 0, cosf(camDat->angle_x));
	Vector3 tmp_right = Vector3(cross((Vector3) {0.f, 1.f, 0.f}, forward));
	direction += forward + tmp_right;

	eye_pos = (Point3) (direction); //+  (Vector3) eye_pos);


	Vector3 tmp_eye = Vector3(
		cosf(camDat->angle_x) * sinf(camDat->angle_y),
        sinf(camDat->angle_x),
        cosf(camDat->angle_x) * cosf(camDat->angle_y)
	);
	eye_dir = (Point3) normalize(tmp_eye);

	Vector3 right = cross((Vector3) {0.f, 1.f, 0.f}, (Vector3) eye_dir);
	Vector3 up = cross((Vector3) eye_dir, right);
	Vector3 dir = direction + (Vector3) eye_dir;

	viewMatrix = Matrix4::lookAt((Point3) direction, (Point3) dir, up);
	modelViewMatrix = viewMatrix;

	rotX = Matrix4::rotationX(DEGTORAD(0)); //camDat->angle_x));
	rotY = Matrix4::rotationY(DEGTORAD(0)); // camDat->angle_y));
	modelMatrix = rotX * rotY;
	modelMatrixIT = inverse(modelMatrix);
	modelViewMatrix = transpose(viewMatrix * modelMatrix);

	objEyePos = modelMatrixIT * eye_pos;
	objLightPos = modelMatrixIT * lightPos;

	rsxAddressToOffset(&mesh->vertices[0].pos, &offset);
	rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_POS, 0, offset, sizeof(S3DVertex), 3, GCM_VERTEX_DATA_TYPE_F32, GCM_LOCATION_RSX);

	// rsxAddressToOffset(&mesh->vertices[0].nrm,&offset);
	// rsxBindVertexArrayAttrib(context,GCM_VERTEX_ATTRIB_NORMAL,0,offset,sizeof(S3DVertex),3,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);

	// rsxAddressToOffset(&mesh->vertices[0].u,&offset);
	// rsxBindVertexArrayAttrib(context,GCM_VERTEX_ATTRIB_TEX0,0,offset,sizeof(S3DVertex),2,GCM_VERTEX_DATA_TYPE_F32,GCM_LOCATION_RSX);

	rsxLoadVertexProgram(context, vpo,vp_ucode);
	rsxSetVertexProgramParameter(context, vpo, projMatrix, (float*) &P);
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



int main()
{
	cam cameraSticks = {0.f, 0.f, 0.f, 0.f};
	padInfo padinfo;
	padData paddata;
	void *host_addr = memalign(HOST_ADDR_ALIGNMENT,HOSTBUFFER_SIZE);

	printf("rsxtest started...\n");

	init_screen(host_addr,HOSTBUFFER_SIZE);
	ioPadInit(7);
	init_shader();
	init_texture();

	sphere = createSphere(3.0f,32,32);
	donut = createDonut(3.0f,1.5f,32,32);
	cube = createCube(5.0f);

	atexit(program_exit_callback);
	sysUtilRegisterCallback(0,sysutil_exit_callback,NULL);



	setDrawEnv();
	setRenderTarget(curr_fb);

	running = 1;
	while(running) {
		sysUtilCheckCallback();

		ioPadGetInfo(&padinfo);
		for(int i=0; i < MAX_PADS; i++){
			if(padinfo.status[i]){
				ioPadGetData(i, &paddata);

				cameraSticks.pos_x -= 0.1 * normalizeAnalogSticks((f32) paddata.ANA_L_V);
				cameraSticks.pos_z -= 0.1 * normalizeAnalogSticks((f32) paddata.ANA_L_H);
				cameraSticks.angle_y -= 0.2 * normalizeAnalogSticks((f32) paddata.ANA_R_H);
				cameraSticks.angle_x += 0.2 * normalizeAnalogSticks((f32) paddata.ANA_R_V);

				if(paddata.BTN_CROSS)
					goto done;
			}

		}
		
		drawFrame(&cameraSticks);

		P = transpose(Matrix4::perspective(DEGTORAD(90.f),aspect_ratio,1.0f,3000.0f));

		flip();
	}

done:
    printf("rsxtest done...\n");

    program_exit_callback();
    return 0;
}
