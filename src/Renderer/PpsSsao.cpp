/*
The file contains functions and vars used for the deferred shading/post-processing stage/SSAO pass.
*/

#include "Renderer.h"
#include "Resource.h"
#include "Texture.h"
#include "Scene.h"
#include "Fbo.h"
#include "Camera.h"

namespace R {
namespace Pps {
namespace Ssao {


//=====================================================================================================================================
// VARS                                                                                                                               =
//=====================================================================================================================================
bool enabled = true;

static Fbo pass0Fbo, pass1Fbo, pass2Fbo;

float renderingQuality = 0.75; // the renderingQuality of the SSAO fai. Chose low so it can blend
float bluringQuality = 1.0;

static uint w, h;
static uint bw, bh;

Texture pass0Fai, pass1Fai, fai;

static ShaderProg ssaoSProg, blurSProg, blurSProg2;

static Texture* noiseMap;


//=====================================================================================================================================
// initBlurFbos                                                                                                                       =
//=====================================================================================================================================
static void initBlurFbos()
{
	// create FBO
	pass1Fbo.create();
	pass1Fbo.bind();

	// inform in what buffers we draw
	pass1Fbo.setNumOfColorAttachements(1);

	// create the texes
	pass1Fai.createEmpty2D( bw, bh, GL_ALPHA8, GL_ALPHA );
	pass1Fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	pass1Fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pass1Fai.getGlId(), 0 );

	// test if success
	if( !pass1Fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO blur FBO" );

	// unbind
	pass1Fbo.unbind();



	// create FBO
	pass2Fbo.create();
	pass2Fbo.bind();

	// inform in what buffers we draw
	pass2Fbo.setNumOfColorAttachements(1);

	// create the texes
	fai.createEmpty2D( bw, bh, GL_ALPHA8, GL_ALPHA );
	fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fai.getGlId(), 0 );

	// test if success
	if( !pass2Fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO blur FBO" );

	// unbind
	pass2Fbo.unbind();
}


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	w = R::Pps::Ssao::renderingQuality * R::w;
	h = R::Pps::Ssao::renderingQuality * R::h;
	bw = w * bluringQuality;
	bh = h * bluringQuality;

	// create FBO
	pass0Fbo.create();
	pass0Fbo.bind();

	// inform in what buffers we draw
	pass0Fbo.setNumOfColorAttachements(1);

	// create the texes
	pass0Fai.createEmpty2D( w, h, GL_ALPHA8, GL_ALPHA );
	pass0Fai.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	pass0Fai.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// attach
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pass0Fai.getGlId(), 0 );

	// test if success
	if( !pass0Fbo.isGood() )
		FATAL( "Cannot create deferred shading post-processing stage SSAO pass FBO" );

	// unbind
	pass0Fbo.unbind();


	// init shaders
	ssaoSProg.customLoad( "shaders/PpsSsao.glsl" );

	// load noise map and disable temporaly the texture compression and enable mipmapping
	bool texCompr = R::textureCompression;
	bool mipmaping = R::mipmapping;
	R::textureCompression = false;
	R::mipmapping = true;
	noiseMap = Rsrc::textures.load( "gfx/noise3.tga" );
	noiseMap->texParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	noiseMap->texParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );
	//noise_map->texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//noise_map->texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	R::textureCompression = texCompr;
	R::mipmapping = mipmaping;

	// blur FBO
	initBlurFbos();
	blurSProg.customLoad( "shaders/PpsSsaoBlur.glsl", ("#define _PPS_SSAO_PASS_0_\n#define PASS0_FAI_WIDTH " + Util::floatToStr(w) + "\n").c_str() );
	blurSProg2.customLoad( "shaders/PpsSsaoBlur.glsl", ("#define _PPS_SSAO_PASS_1_\n#define PASS1_FAI_HEIGHT " + Util::floatToStr(bh) + "\n").c_str() );
}


/*
=======================================================================================================================================
runPass                                                                                                                               =
=======================================================================================================================================
*/
void runPass( const Camera& cam )
{
	pass0Fbo.bind();

	R::setViewport( 0, 0, w, h );

	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );

	// fill SSAO FAI
	ssaoSProg.bind();
	glUniform2fv( ssaoSProg.getUniVar("camerarange")->getLoc(), 1, &(Vec2(cam.getZNear(), cam.getZFar()))[0] );
	ssaoSProg.locTexUnit( ssaoSProg.getUniVar("msDepthFai")->getLoc(), R::Ms::depthFai, 0 );
	ssaoSProg.locTexUnit( ssaoSProg.getUniVar("noiseMap")->getLoc(), *noiseMap, 1 );
	ssaoSProg.locTexUnit( ssaoSProg.getUniVar("msNormalFai")->getLoc(), R::Ms::normalFai, 2 );
	R::DrawQuad( 0 ); // Draw quad

	/*glBindFramebuffer( GL_READ_FRAMEBUFFER, pass0Fbo.getGlId() );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, pass1Fbo.getGlId() );
	glBlitFramebuffer( 0, 0, w, h, 0, 0, bw, bh, GL_COLOR_BUFFER_BIT, GL_NEAREST);*/

	R::setViewport( 0, 0, bw, bh );

	// second pass. blur
	pass1Fbo.bind();
	blurSProg.bind();
	blurSProg.locTexUnit( blurSProg.getUniVar("tex")->getLoc(), pass0Fai, 0 );
	R::DrawQuad( 0 ); // Draw quad

	// third pass. blur
	pass2Fbo.bind();
	blurSProg2.bind();
	blurSProg2.locTexUnit( blurSProg2.getUniVar("tex")->getLoc(), pass1Fai, 0 );
	R::DrawQuad( 0 ); // Draw quad


	// end
	Fbo::unbind();
}


}}} // end namespaces