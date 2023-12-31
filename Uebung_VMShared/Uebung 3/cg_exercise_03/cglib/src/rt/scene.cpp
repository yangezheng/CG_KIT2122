#include <cglib/rt/scene.h>

#include <cglib/rt/epsilon.h>
#include <cglib/rt/light.h>
#include <cglib/rt/object.h>
#include <cglib/rt/raytracing_parameters.h>
#include <cglib/rt/texture.h>

#include <cglib/rt/transform.h>

#include <cglib/core/camera.h>
#include <cglib/core/image.h>

#include <sstream>
#include <random>

Scene::~Scene()
{
}

void Scene::
set_active_camera()
{
	if(camera)
		camera->set_active();
}

TextureScene::TextureScene(RaytracingParameters & params)
{
    init_camera(params);
    init_scene(params);
}

void TextureScene::init_scene(RaytracingParameters const& params)
{
    objects.clear();
    lights.clear();
    textures.clear();

    textures.insert({"earth", std::make_shared<ImageTexture>("assets/earth.tga", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 2.2f)});
	textures["earth"]->create_mipmap();
    textures.insert({"floor", std::make_shared<ImageTexture>("assets/checker.tga", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 2.2f)});
	textures["floor"]->create_mipmap();

    objects.emplace_back(create_sphere(glm::vec3(2.5f, 0.f, 0.f), 1.f));
    objects.back()->material->k_d = textures["earth"];
    cg_assert(objects.back()->material->k_d);
    objects.emplace_back(create_sphere(glm::vec3(-2.5f, 0.f, 0.f), 1.f));
    objects.back()->material->k_d = textures["earth"];
    objects.emplace_back((create_plane(
        glm::vec3(0.f, -2.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 0.f, -1.f),
        glm::vec2(4.f))));
    objects.back()->material->k_d = textures["floor"];

    lights.emplace_back(new Light(glm::vec3(0.f, 2.f, 4.f), glm::vec3(1.f)));
}

void TextureScene::refresh_scene(RaytracingParameters const& params)
{
    for (auto &tex : textures) {
        tex.second->filter_mode = params.get_tex_filter_mode();
        tex.second->wrap_mode = params.get_tex_wrap_mode();
    }
}

void TextureScene::init_camera(RaytracingParameters& params)
{
    camera = std::make_shared<LookAroundCamera>(
        glm::vec3(0.f, 0.f, 10.f),
        glm::vec3(0.f, 0.f, -1.f),
        params.eye_separation,
		params.focal_distance);
}

TexturedSphereScene::TexturedSphereScene(RaytracingParameters & params)
{
    init_camera(params);
    init_scene(params);
}

void TexturedSphereScene::init_scene(RaytracingParameters const& params)
{
    objects.clear();
    lights.clear();
    textures.clear();

    textures.insert({"stone-diffuse", std::make_shared<ImageTexture>("assets/photosculpt-graystonewall-diffuse.jpg", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 2.2f)});
    textures.insert({"stone-normal", std::make_shared<ImageTexture>("assets/photosculpt-graystonewall-normal.jpg", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 1.f)});
	textures["stone-diffuse"]->create_mipmap();
	textures["stone-normal"]->create_mipmap();

    objects.emplace_back(create_sphere(glm::vec3(0.f, 0.f, 0.f), 2.f, glm::vec2(5.f, 5.f)));
	//objects.back()->set_transform_object_to_world(glm::rotate(float(M_PI/4.0), glm::normalize(glm::vec3(-0.8f, 0.f, 0.5f))));
    objects.back()->material->k_d = textures["stone-diffuse"];
    objects.back()->material->normal = textures["stone-normal"];

    textures.insert({"concrete-diffuse", std::make_shared<ImageTexture>("assets/concrete_diffuse.jpg", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 2.2f)});
    textures.insert({"concrete-normal", std::make_shared<ImageTexture>("assets/concrete_normal.jpg", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 1.f)});
	textures["concrete-diffuse"]->create_mipmap();
	textures["concrete-normal"]->create_mipmap();
	
    objects.emplace_back((create_plane(
        glm::vec3(0.f, -2.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 0.f, -1.f),
        glm::vec2(4.f))));
	objects.back()->material->k_d    = textures["concrete-diffuse"];
	objects.back()->material->normal = textures["concrete-normal"];

    lights.emplace_back(new Light(glm::vec3(3.f, 4.f, 2.f), glm::vec3(10.f)));
    lights.emplace_back(new Light(glm::vec3(0.f, 4.f, 5.f), glm::vec3(1.f)));
}

void TexturedSphereScene::refresh_scene(RaytracingParameters const& params)
{
    for (auto &tex : textures) {
        tex.second->filter_mode = params.get_tex_filter_mode();
        tex.second->wrap_mode = params.get_tex_wrap_mode();
    }
}

void TexturedSphereScene::init_camera(RaytracingParameters& params)
{
    camera = std::make_shared<LookAroundCamera>(
        glm::vec3(3.f, 4.f, 5.f),
        glm::vec3(0.f, 0.f, 0.f),
        params.eye_separation,
		params.focal_distance);
}

AliasingPlaneScene::AliasingPlaneScene(RaytracingParameters & params)
{
    init_camera(params);
    init_scene(params);
}

void AliasingPlaneScene::init_scene(RaytracingParameters const& params)
{
    objects.clear();
    lights.clear();
    textures.clear();

    //textures.insert({"texture", std::make_shared<ImageTexture>("assets/aliasing.jpg", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 2.2f)});
    textures.insert({"texture", std::make_shared<ImageTexture>("assets/checker.png", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 2.2f)});
	textures["texture"]->create_mipmap();

	const float scale = 10.f;
    objects.emplace_back(create_plane(
        glm::vec3(-5.f, 0.f, 5.0f),
        glm::normalize(glm::vec3(0.f, 1.f, 0.0f)),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec2(scale)));
	objects.back()->material->k_d    = textures["texture"];

    lights.emplace_back(new Light(glm::vec3(0.f, 1000.f, 0.f), glm::vec3(500000.f)));
}

void AliasingPlaneScene::refresh_scene(RaytracingParameters const& params)
{
    for (auto &tex : textures) {
        tex.second->filter_mode = params.get_tex_filter_mode();
        tex.second->wrap_mode = params.get_tex_wrap_mode();
    }
}

void AliasingPlaneScene::init_camera(RaytracingParameters& params)
{
    camera = std::make_shared<LookAroundCamera>(
		glm::vec3(10.f, 7.0f, 10.f),
        glm::vec3(0.f, 0.f, 0.f),
        params.eye_separation,
		params.focal_distance);
}

PoolTableScene::PoolTableScene(RaytracingParameters & params)
{
    init_camera(params);
    init_scene(params);
}


void PoolTableScene::init_scene(RaytracingParameters const& params)
{
    objects.clear();
    lights.clear();
    textures.clear();

    for (uint32_t i = 1; i <= 15; ++i)
	{
		std::stringstream path, name;
		path << "assets/PoolBalluv" << i << ".png";
		name << "ball" << i;
		textures.insert({name.str().c_str(), std::make_shared<ImageTexture>(path.str().c_str(), params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 2.2f)});
		textures[name.str().c_str()]->create_mipmap();
	}

	textures.insert({"table", std::make_shared<ImageTexture>("assets/pool_table.tga", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 2.2f)});
	textures["table"]->create_mipmap();

    float startX = 0.f;
    uint32_t width = 1;

	std::minstd_rand rng_engine(1337);
	std::uniform_real_distribution<float> uniform_dist_dir(0, 1);

	auto rand = [&]() { return uniform_dist_dir(rng_engine); };
    while (width <= 5)
	{
		for (uint32_t j = 0; j < width; ++j) {
			glm::vec3 pos((startX + j) * 2, 0.f, width * 2);
			objects.emplace_back(create_sphere(glm::vec3(0.0), 1.f));

			glm::vec3 dir = glm::vec3(0.0f);
			dir[0] = rand();
			dir[1] = rand();
			dir[2] = rand();

			objects.back()->set_transform_object_to_world(
					glm::translate(glm::mat4(1.0f), pos) *
					glm::rotate(glm::mat4(1.0f), float(2.0f * M_PI * rand()),
						glm::normalize(dir * 2.0f - glm::vec3(1.0f))
					));
		}

		width++;
		startX -= 0.5f;
	}

    for (uint32_t i = 0; i < 15; ++i)
	{
		objects[i]->material->k_s.reset(new ConstTexture(glm::vec3(1.0f)));
		objects[i]->material->n = 200.f;

		objects[i]->material->k_r.reset(new ConstTexture(glm::vec3(0.05f)));
	}
		
	//     0
	//  1  2
	//  3  4  5
	//  6  7  8  9
	// 10 11 12 13 14

	objects[0]->material->k_d = textures["ball9"];

	objects[1]->material->k_d = textures["ball7"];
	objects[2]->material->k_d = textures["ball12"];
	
	objects[3]->material->k_d = textures["ball15"];
	objects[4]->material->k_d = textures["ball8"];
	objects[5]->material->k_d = textures["ball1"];
	
	objects[6]->material->k_d = textures["ball6"];
	objects[7]->material->k_d = textures["ball10"];
	objects[8]->material->k_d = textures["ball3"];
	objects[9]->material->k_d = textures["ball14"];
	
	objects[10]->material->k_d = textures["ball11"];
	objects[11]->material->k_d = textures["ball2"];
	objects[12]->material->k_d = textures["ball13"];
	objects[13]->material->k_d = textures["ball4"];
	objects[14]->material->k_d = textures["ball5"];

	objects.emplace_back(create_quad(
				glm::vec3(0.f, -1.f, 0.f),
				glm::vec3(0.f, 1.f, 0.f),
				glm::vec3(80.f, 0.f, 0.f),
				glm::vec3(0.f, 0.f, -80.f),
				glm::vec2(4.f)));


    objects.back()->material->k_d = textures["table"];

	textures.insert({"envmap",  std::make_shared<ImageTexture>("assets/appartment.jpg", NEAREST, REPEAT)});
	textures["envmap"]->create_mipmap();
	env_map = textures["envmap"].get();
	
    lights.emplace_back(new Light(glm::vec3(0.f, 4.f, 4.f), glm::vec3(10.f)));
    lights.emplace_back(new Light(glm::vec3(-10.f, 4.f, 4.f), glm::vec3(10.f)));
    lights.emplace_back(new Light(glm::vec3(0.f, 4.f, 16.f), glm::vec3(10.f)));
}

void PoolTableScene::refresh_scene(RaytracingParameters const& params)
{
    for (auto &tex : textures) {
        tex.second->filter_mode = params.get_tex_filter_mode();
        tex.second->wrap_mode = params.get_tex_wrap_mode();
    }
}

void PoolTableScene::init_camera(RaytracingParameters& params)
{
    camera = std::make_shared<LookAroundCamera>(
        glm::vec3(0.f, 12.f, 10.f),
        glm::vec3(0.f, 0.f, 7.f),
        params.eye_separation,
		params.focal_distance);
}

GoBoardScene::GoBoardScene(RaytracingParameters & params)
{
    init_camera(params);
    init_scene(params);
}


void GoBoardScene::init_scene(RaytracingParameters const& params)
{
    objects.clear();
    lights.clear();
    textures.clear();

	// floor
    objects.emplace_back(create_quad(
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 0.f, 100.f),
        glm::vec3(100.f, 0.f, 0.f),
		glm::vec2(10000.0f, 10000.0f)));
	textures.insert({"go_board_diffuse", std::make_shared<ImageTexture>("assets/go_board_diffuse.png", params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 2.2f)});
	textures.insert({"go_board_normal",  std::make_shared<ImageTexture>("assets/go_board_normal.png",  params.get_tex_filter_mode(), params.get_tex_wrap_mode(), 1.f)});
	textures["go_board_diffuse"]->create_mipmap();
	textures["go_board_normal"]->create_mipmap();
    objects.back()->material->k_d = textures["go_board_diffuse"];
    objects.back()->material->k_r = std::shared_ptr<ConstTexture>(new ConstTexture(glm::vec3(0.10f)));
    objects.back()->material->normal = textures["go_board_normal"];

	using glm::translate;
	using glm::scale;
	using glm::vec3;
	using glm::vec4;

	const char *board =
		"..................."
		"...............2..."
		"..2.......22.22.1.."
		".1.2.2.....1.1.1..."
		"..............2.1.."
		"..1............121."
		"................21."
		"...........11...22."
		"..1.........22.2..."
		"...............211."
		"............12112.."
		".............1..2.."
		".............21.1.."
		"........1..22..1..."
		"..1111....2.12...1."
		".221212.....1.2.1.."
		".21222111.2.1..221."
		".212.12....2......."
		"..................."
		;

	std::minstd_rand rng_engine(1337);
	std::uniform_real_distribution<float> uniform_dist_dir(0, 1);

	auto rand = [&]() { return uniform_dist_dir(rng_engine); };

	const int go_size = 19;
	// sphere
	for(int y = 0; y < go_size; y++) {
		for(int x = 0; x < go_size; x++) {
			if(board[y * go_size + x] == '.')
				continue;

			float r = 2.3f + rand() * 0.01f;
			objects.emplace_back(create_sphere(vec3(0.0f), r));
			float rnd1 = rand();
			float rnd2 = rand();
			objects.back()->set_transform_object_to_world(
					translate(glm::mat4(1.0), vec3(float(x - go_size / 2) * 4.78515625 + (rnd1 - 0.5) * 0.25,
							r / 2.0f,
							float(y - go_size / 2) * 4.78515625f + (rnd2 - 0.5) * 0.25))
					* scale(glm::mat4(1.0), vec3(1.f, 0.5f, 1.f)));

			objects.back()->material->k_d = std::make_shared<ConstTexture>(
					board[y * go_size + x] == '1'
					? vec3(0.025f)
					: vec3(0.9f));

			objects.back()->material->k_r = std::make_shared<ConstTexture>(
					board[y * go_size + x] == '1'
					? vec3(0.015f)
					: vec3(0.08f));
		}
	}

	lights.emplace_back(new Light(glm::vec3(-0.f, 100.f, 0.f), 100.f*glm::vec3(50.f, 40.f, 40.f)));
	lights.emplace_back(new Light(glm::vec3(-200.f, 300.f, 0.f), 25.f*glm::vec3(50.f, 50.f, 50.f)));
	lights.emplace_back(new Light(glm::vec3(-150.f, 300.f, 200.f), 25.f*glm::vec3(50.f, 50.f, 50.f)));

	textures.insert({"envmap",  std::make_shared<ImageTexture>("assets/warehouse.jpg", NEAREST, REPEAT)});
	textures["envmap"]->create_mipmap();
	env_map = textures["envmap"].get();
}

void GoBoardScene::refresh_scene(RaytracingParameters const& params)
{
    for (auto &tex : textures) {
        tex.second->filter_mode = params.get_tex_filter_mode();
        tex.second->wrap_mode = params.get_tex_wrap_mode();
    }
}

void GoBoardScene::init_camera(RaytracingParameters& params)
{
	camera = std::make_shared<FreeFlightCamera>(
			glm::vec3(44.24, 12.1067, 53.164),
			glm::normalize(glm::vec3(-0.513036, -0.309281, -0.800712)),
			params.eye_separation,
		params.focal_distance);
}


FourierScene::FourierScene(RaytracingParameters& params)
{
    init_scene(params);
}

void FourierScene::init_scene(RaytracingParameters const& params)
{
}

void FourierScene::refresh_scene(RaytracingParameters const& params)
{
	if(textures.size() > 0)
		return;

	std::cout << "Computing reconstructed image (this might take a while)... " << std::flush;

	std::shared_ptr<Image> img_spectrum = std::make_shared<Image>();
	img_spectrum->load_pfm("assets/mystery.pfm");
	textures.insert({"img_spectrum", std::make_shared<ImageTexture>(
				*img_spectrum.get(), NEAREST, ZERO)});

	glm::vec4 *spec_data = img_spectrum->getPixels();

	// calculate amplitude
	std::shared_ptr<Image> amplitude = std::make_shared<Image>(
			img_spectrum->getWidth(), img_spectrum->getHeight());
	glm::vec4 *ampl_data = amplitude->getPixels();
	for(int i = 0; i < img_spectrum->getWidth() * img_spectrum->getHeight(); i++)
	{
		float re = spec_data[i].r;
		float im = spec_data[i].g;
		float a  = sqrtf(re*re+im*im);

		ampl_data[i] = glm::vec4(a, a, a, 1.0f);
	}

	textures.insert({"amplitude", std::make_shared<ImageTexture>(
				*amplitude.get(), NEAREST, ZERO)});


	// calculate phase
	std::shared_ptr<Image> phase = std::make_shared<Image>(
			img_spectrum->getWidth(), img_spectrum->getHeight());
	glm::vec4 *phase_data = phase->getPixels();
	for(int i = 0; i < img_spectrum->getWidth() * img_spectrum->getHeight(); i++)
	{
		float re = spec_data[i].r;
		float im = spec_data[i].g;
		float ph = (std::atan2(im, re)+M_PI)/(2.0f*M_PI);

		phase_data[i] = glm::vec4(ph, ph, ph, 1.0f);
	}

	textures.insert({"phase", std::make_shared<ImageTexture>(
				*phase.get(), NEAREST, ZERO)});

	int sx = img_spectrum->getWidth(), sy = img_spectrum->getHeight();
	std::vector<std::complex<float>> spectrum (sx*sy, std::complex<float>(0.0f, 0.0f));
	Image::image_to_complex(*img_spectrum, &spectrum, false);

	// reconstruct the loaded spectrum and store it as png
	std::vector<std::complex<float>> reconstruction(sx*sy, std::complex<float>(0.0f, 0.0f));
	DiscreteFourier2D::reconstruct(sx, sy, spectrum.data(), reconstruction.data());
	Image img_rec(sx, sy);
	Image::complex_to_image(reconstruction, &img_rec, true);

	textures.insert({"reconstructed", std::make_shared<ImageTexture>(img_rec, NEAREST, ZERO)});

	std::cout << "done" << std::endl;
}


