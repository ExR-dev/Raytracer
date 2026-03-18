#include "Utils.h"
#include "Vec3.h"
#include "Graphics.h"
#include "Shapes.h"

#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "rapidjson.h"
#include "prettywriter.h"

#include <iostream>
#include <string>
#include <format>
#include <cmath>

struct Cam
{
	float fov;
	bool perspective;
	float speed;

	Vec3
		origin,
		fwd, right, up;


	Cam(float fov, bool perspective, float speed, const Vec3& origin, const Vec3& fwd) :
		fov(fov), perspective(perspective), speed(speed), origin(origin), fwd(fwd)
	{
		UpdateRotation();
	}

	void UpdateRotation()
	{
		fwd.Normalize();

		right = fwd.Cross({ 0, -1, 0 });
		right.Normalize();

		up = fwd.Cross(right);
		up.Normalize();
	}
};


static bool EditMaterial(Material &mat)
{
	bool isEdited = false;

	if (ImGui::TreeNode("Material"))
	{
		ImGuiColorEditFlags flags = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float;
		isEdited |= ImGui::ColorEdit4("Albedo", &mat.albedo.x, flags);
		isEdited |= ImGui::ColorEdit4("Specular", &mat.specular.x, flags);
		isEdited |= ImGui::ColorEdit4("Emission", &mat.emission.x, flags);
		isEdited |= ImGui::ColorEdit3("Absorption", &mat.absorption.x, flags);
		isEdited |= ImGui::DragFloat("Absorption Offset", &mat.absorption.w, 0.01f);

		isEdited |= ImGui::DragFloat("Albedo Reflectivity", &mat.albedoReflectivity, 0.01f, 0.0f, 1.0f);
		isEdited |= ImGui::DragFloat("Specular Reflectivity", &mat.specularReflectivity, 0.01f, 0.0f, 1.0f);
		isEdited |= ImGui::DragFloat("Reflective Index", &mat.reflectiveIndex, 0.01f, 0.0f);

		ImGui::TreePop();
	}

	return isEdited;
}

static bool EditShape(Shape &shape, bool &remove)
{
	bool isEdited = false;
	bool isOpen = false;

	if (auto* aabb = dynamic_cast<ShapeAABB*>(&shape))
	{
		isOpen = ImGui::TreeNode("AABB");

		if (isOpen)
		{
			isEdited |= ImGui::DragFloat3("Min", &aabb->min.x, 0.1f);
			isEdited |= ImGui::DragFloat3("Max", &aabb->max.x, 0.1f);
		}
	}
	else if (auto* obb = dynamic_cast<ShapeOBB*>(&shape))
	{
		isOpen = ImGui::TreeNode("OBB");

		if (isOpen)
		{
			isEdited |= ImGui::DragFloat3("Center", &obb->center.x, 0.1f);
			isEdited |= ImGui::DragFloat3("Half Extents", &obb->halfLength.x, 0.1f);
			isEdited |= ImGui::DragFloat3("X-Axis", &obb->xAxis.x, 0.1f);
			isEdited |= ImGui::DragFloat3("Y-Axis", &obb->yAxis.x, 0.1f);
			isEdited |= ImGui::DragFloat3("Z-Axis", &obb->zAxis.x, 0.1f);
		}
	}
	else if (auto* sphere = dynamic_cast<ShapeSphere*>(&shape))
	{
		isOpen = ImGui::TreeNode("Sphere");

		if (isOpen)
		{
			isEdited |= ImGui::DragFloat3("Position", &sphere->pos.x, 0.1f);
			isEdited |= ImGui::DragFloat("Radius", &sphere->rad, 0.1f);
		}
	}
	else if (auto* tri = dynamic_cast<ShapeTri*>(&shape))
	{
		isOpen = ImGui::TreeNode("Triangle");

		if (isOpen)
		{
			isEdited |= ImGui::DragFloat3("Vertex 1", &tri->v1.x, 0.1f);
			isEdited |= ImGui::DragFloat3("Vertex 2", &tri->v2.x, 0.1f);
			isEdited |= ImGui::DragFloat3("Vertex 3", &tri->v3.x, 0.1f);
		}
	}
	else if (auto* plane = dynamic_cast<ShapePlane*>(&shape))
	{
		isOpen = ImGui::TreeNode("Plane");

		if (isOpen)
		{
			isEdited |= ImGui::DragFloat3("Center", &plane->center.x, 0.1f);
			isEdited |= ImGui::DragFloat3("Normal", &plane->normal.x, 0.1f);
		}
	}

	if (isOpen)
	{
		isEdited |= EditMaterial(shape.mat);

		if (ImGui::Button("Remove"))
			remove = true;

		ImGui::TreePop();
	}

	return isEdited;
}


int main()
{
	if (!sf::Shader::isAvailable())
		return 1;

	unsigned int
		w = 1280, 
		h = 720,
		dim = w * h;

	unsigned int nextSnapshot = 0;

	// Build Scene
	Cam cam(
		75.0f, true, 5.0f,
		Vec3(0.0, 5.0, -10.0),
		Vec3(0.0, -0.531709431, 1.0).Normalize()
	);

	sf::RenderWindow window(sf::VideoMode({ w, h }), "SFML Window", sf::State::Windowed);
	if (!ImGui::SFML::Init(window))
		std::cout << "Failed to initialize ImGui-SFML" << std::endl;

	Color* render = new Color[dim];
	for (int i = 0; i < dim; i++)
		render[i] = Color();

	sf::Image
		renderImg({ w, h }, sf::Color::Black),
		displayImg({ w, h }, sf::Color::Black);

	sf::RenderTexture renderTex({ w, h });

	sf::Texture tex(sf::Vector2u(w, h)), displayTex(sf::Vector2u(w, h));

	sf::Sprite sprite(tex), displaySprite(displayTex);

	sf::Shader shader;
	if (!shader.loadFromFile("RaytracerShader.frag", sf::Shader::Type::Fragment))
	{
		std::cerr << "Failed to load shader" << std::endl;
		return -1;
	}

	sf::Clock clock, imClock;
	double lT = 0.0, tT = 0.0, dT = 0.0;

	sf::Vector2i deltas, windowPos, windowSize({(int)w, (int)h});

	bool cumulativeLighting, realRender, randomizeSampleDir, keepConstant, giveControl, disableLighting, viewBounds;
	unsigned int perPixelSamples, maxBounces;

	{
		keepConstant = false;
		giveControl = false;
		cam.fov = 75.0f;

		cumulativeLighting = true;
		realRender = false;
		randomizeSampleDir = true;
		disableLighting = false;
		viewBounds = false;
		perPixelSamples = 8;
		maxBounces = 6;
	}

	shader.setUniform("imgW", (int)w);
	shader.setUniform("imgH", (int)h);
	shader.setUniform("samples", (int)perPixelSamples);
	shader.setUniform("maxBounces", (int)maxBounces);

	std::vector<Shape*> shapes;
	{
		// Example AABB
		auto* aabb1 = new ShapeAABB;
		aabb1->min = sf::Glsl::Vec3(-25.0, 0.0, -20.0);
		aabb1->max = sf::Glsl::Vec3(25.0, 15.0, -18.0);
		shapes.push_back(aabb1);

		// Example Sphere
		auto* sphere1 = new ShapeSphere;
		sphere1->pos = sf::Glsl::Vec3(1.0, 3.0, 0.0);
		sphere1->rad = 3.0f;
		shapes.push_back(sphere1);

		// Example Plane
		auto* plane1 = new ShapePlane;
		plane1->center = sf::Glsl::Vec3(0.0, -1.0, 0.0);
		plane1->normal = sf::Glsl::Vec3(0.0, 1.0, 0.0);
		shapes.push_back(plane1);
	}

	// Bind all shapes to shader
	BindShapes(shapes, shader);

	unsigned int
		cumulativeFrameCount = 0,
		totFrames = 0;

	bool 
		guiFocused = false, 
		guiHovered = false;

	while (window.isOpen())
	{
		lT = tT;
		tT = clock.getElapsedTime().asSeconds();
		dT = tT - lT;

		bool hasMoved = false;

		while (std::optional<sf::Event> optEvent = window.pollEvent())
		{
			sf::Event event = optEvent.value();

			ImGui::SFML::ProcessEvent(window, event);

			if (event.is<sf::Event::MouseButtonPressed>())
			{
				auto eventSubtype = event.getIf<sf::Event::MouseButtonPressed>();

				if (eventSubtype->button == sf::Mouse::Button::Right)
				{
					giveControl = !giveControl;
				}
			}

			if (event.is<sf::Event::MouseWheelScrolled>() && !guiFocused && !guiHovered)
			{
				auto eventSubtype = event.getIf<sf::Event::MouseWheelScrolled>();

				if (eventSubtype->delta != 0.0f)
				{
					float lFov = cam.fov;
					cam.fov = std::clamp(cam.fov - eventSubtype->delta, 0.01f, 179.99f);

					if (abs(cam.fov - lFov) > 0.000001)
						hasMoved = true;
				}
			}

			if (event.is<sf::Event::KeyPressed>() && !guiFocused)
			{
				auto eventSubtype = event.getIf<sf::Event::KeyPressed>();

				if (eventSubtype->code == sf::Keyboard::Key::Enter)
				{
					sf::Image snapshotImage;

					if (realRender)
						snapshotImage = displayImg;
					else
						snapshotImage = renderTex.getTexture().copyToImage();

					nextSnapshot = utils::FirstUnusedSnapshot(nextSnapshot);
					std::string filename = "Snapshots/Snapshot " + std::to_string(nextSnapshot) + ".png";

					if (!snapshotImage.saveToFile(filename))
						std::cout << "Saving Failed!";
				}
				else if (eventSubtype->code == sf::Keyboard::Key::L)
				{
					disableLighting = !disableLighting;
					hasMoved = true;
				}
				else if (eventSubtype->code == sf::Keyboard::Key::B)
				{
					viewBounds = !viewBounds;
					hasMoved = true;
				}
				else if (eventSubtype->code == sf::Keyboard::Key::R)
				{
					randomizeSampleDir = !randomizeSampleDir;
				}
				else if (eventSubtype->code == sf::Keyboard::Key::T)
				{
					keepConstant = !keepConstant;
				}
				else if (eventSubtype->code == sf::Keyboard::Key::C)
				{
					cumulativeFrameCount = 0;
					cumulativeLighting = !cumulativeLighting;
					hasMoved = true;
				}
				else if (eventSubtype->code == sf::Keyboard::Key::E)
				{
					realRender = !realRender;
					cumulativeFrameCount = 0;
					hasMoved = true;
				}
				else if (eventSubtype->code == sf::Keyboard::Key::V)
					hasMoved = true;
			}

			if (event.is<sf::Event::Resized>())
			{
				auto eventSubtype = event.getIf<sf::Event::Resized>();

				w = eventSubtype->size.x;
				h = eventSubtype->size.y;
				dim = w * h;

				window.setView(sf::View({ (float)w * 0.5f, (float)h * 0.5f }, { (float)w, (float)h }));

				delete[] render;
				render = new Color[dim];
				for (int i = 0; i < dim; i++)
					render[i] = Color();

				renderImg = sf::Image({ w, h }, sf::Color::Black);
				displayImg = sf::Image({ w, h }, sf::Color::Black);

				renderTex = sf::RenderTexture({ w, h });

				tex = sf::Texture(sf::Vector2u(w, h));
				displayTex = sf::Texture(sf::Vector2u(w, h));

				sprite = sf::Sprite(tex);
				displaySprite = sf::Sprite(displayTex);

				shader.setUniform("imgW", (int)w);
				shader.setUniform("imgH", (int)h);

				windowSize = sf::Vector2i(w, h);

				cumulativeFrameCount = 0;
				hasMoved = true;
			}

			if (event.is<sf::Event::Closed>())
			{
				window.close();
			}
		}

		ImGui::SFML::Update(window, imClock.restart());

		ImGui::Begin("Debug");

		guiFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		guiHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

		if (ImGui::Button("Snapshot"))
		{
			sf::Image snapshotImage;

			if (realRender)
				snapshotImage = displayImg;
			else
				snapshotImage = renderTex.getTexture().copyToImage();

			nextSnapshot = utils::FirstUnusedSnapshot(nextSnapshot);
			std::string filename = "Snapshots/Snapshot " + std::to_string(nextSnapshot) + ".png";

			if (!snapshotImage.saveToFile(filename))
				std::cout << "Saving Failed!";
		}

		ImGui::SameLine();
		if (ImGui::Button("Quit"))
			window.close();

		ImGui::SameLine();
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

		if (ImGui::BeginTabBar("Tabs"))
		{
			if (ImGui::BeginTabItem("Scene"))
			{
				bool isEdited = false;

				enum class ShapeType { AABB, OBB, Sphere, Tri, Plane };
				static ShapeType currSelectedShape = ShapeType::AABB;

				ImGui::Combo("##ShapeType", (int*)&currSelectedShape, "AABB\0OBB\0Sphere\0Triangle\0Plane\0\0");

				ImGui::SameLine();
				if (ImGui::Button("Add"))
				{
					isEdited = true;

					Shape* newShape;
					switch (currSelectedShape)
					{
					case ShapeType::AABB:
						newShape = new ShapeAABB;
						break;
					case ShapeType::OBB:
						newShape = new ShapeOBB;
						break;
					case ShapeType::Sphere:
						newShape = new ShapeSphere;
						break;
					case ShapeType::Tri:
						newShape = new ShapeTri;
						break;
					case ShapeType::Plane:
						newShape = new ShapePlane;
						break;
					default:
						newShape = new ShapeAABB;
					}

					shapes.push_back(newShape);
					BindShapes(shapes, shader);
					hasMoved = true;
				}

				ImGui::BeginChild("Shapes", { 0, 150 }, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY);
				for (int i = 0; i < shapes.size(); ++i)
				{
					Shape* shape = shapes[i];
					ImGui::PushID(shape);

					bool remove = false;
					isEdited |= EditShape(*shape, remove);

					if (remove)
					{
						shapes.erase(std::remove(shapes.begin(), shapes.end(), shape), shapes.end());
						delete shape;
						isEdited = true;
						i--;
					}

					ImGui::PopID();
				}
				ImGui::EndChild();

				if (isEdited)
				{
					BindShapes(shapes, shader);
					hasMoved = true;
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Camera"))
			{
				ImGui::SliderFloat("Speed", &cam.speed, 0.01f, 100.0f);

				if (ImGui::SliderFloat("FOV", &cam.fov, 0.01f, 179.99f))
					hasMoved = true;

				if (ImGui::DragScalarN("Position", ImGuiDataType_Double, &cam.origin.x, 3, 0.1f))
					hasMoved = true;

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Rendering"))
			{
				sf::Vector2u res(w, h);
				if (ImGui::DragScalarN("Resolution", ImGuiDataType_U32, &res.x, 2, 1.0f))
				{
					res.x = std::max(1u, res.x);
					res.y = std::max(1u, res.y);
					window.setSize(res);
				}

				if (ImGui::DragInt("Per Pixel Samples", (int*)&perPixelSamples, 1.0f, 1, 1024))
				{
					shader.setUniform("samples", (int)perPixelSamples);
					cumulativeFrameCount = 0;
					hasMoved = true;
				}

				if (ImGui::DragInt("Max Bounces", (int*)&maxBounces, 1.0f, 1, 64))
				{
					shader.setUniform("maxBounces", (int)maxBounces);
					cumulativeFrameCount = 0;
					hasMoved = true;
				}

				if (ImGui::Checkbox("High-quality Render", &realRender))
				{
					cumulativeFrameCount = 0;
					hasMoved = true;
				}

				if (ImGui::Checkbox("Cumulative Lighting", &cumulativeLighting))
				{
					cumulativeFrameCount = 0;
					hasMoved = true;
				}

				if (ImGui::Checkbox("Randomize Sample Directions", &randomizeSampleDir))
				{
					cumulativeFrameCount = 0;
					hasMoved = true;
				}

				if (ImGui::Checkbox("Keep Sample Directions Constant", &keepConstant))
				{
					cumulativeFrameCount = 0;
					hasMoved = true;
				}

				ImGui::Checkbox("Disable Lighting", &disableLighting);

				ImGui::Checkbox("View Bounds", &viewBounds);

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
			window.close();

		if (giveControl)
		{
			double speedMult = (double)cam.speed * dT * (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ? 3.0 : 1.0);
			speedMult /= (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ? 6.0 : 1.0);

			Vec3 camLOrigin = cam.origin;
			Vec3 camLFwd = cam.fwd;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
				cam.origin += cam.fwd * speedMult;
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
				cam.origin -= cam.fwd * speedMult;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
				cam.origin += cam.right * speedMult;
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
				cam.origin -= cam.right * speedMult;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
				cam.origin += cam.up * speedMult;
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X))
				cam.origin -= cam.up * speedMult;

			sf::Vector2i fixedPoint = windowPos + windowSize.componentWiseDiv({ 2, 2 });

			windowPos = window.getPosition();
			deltas = fixedPoint - sf::Mouse::getPosition();
			if (deltas != sf::Vector2i(0, 0))
				sf::Mouse::setPosition(fixedPoint);

			if (deltas.y != 0)
			{
				float sign = (float)deltas.y * -0.001f;
				int verticality = (sign > 0) ? -1 : 1;

				Vec3 offAngle = cam.fwd - Vec3(0, verticality, 0);
				offAngle.NormalizeApprox();

				cam.fwd = (
					cam.fwd * cos(sign) +
					cam.right.Cross(cam.fwd) * sin(sign) +
					cam.right * cam.right.Dot(cam.fwd) * (1.0f - cos(sign))
					);

				if (offAngle.Dot(cam.fwd) <= 0)
					cam.fwd = Vec3(0, verticality, 0) + offAngle * utils::MINVAL * 100.0;
			}

			if (deltas.x != 0 && abs(cam.fwd.y) != 1.0)
			{
				float sign = (float)deltas.x * -0.001f;

				cam.fwd = {
					cam.fwd.x * cos(sign) + cam.fwd.z * sin(sign),
					cam.fwd.y,
					-cam.fwd.x * sin(sign) + cam.fwd.z * cos(sign)
				};
			}

			cam.UpdateRotation();

			if ((camLOrigin - cam.origin).MagSqr() > 0.000001 || (camLFwd - cam.fwd).MagSqr() > 0.000001)
				hasMoved = true;
		}

		if (cumulativeLighting && hasMoved)
		{
			cumulativeFrameCount = 0;
			renderTex.clear();

			if (realRender)
			{
				for (int i = 0; i < dim; i++)
				{
					render[i] = Color();
					renderImg.setPixel({ i % w, i / w }, { 0, 0, 0, 0 });
				}
			}
		}

		if (realRender && cumulativeFrameCount > 0)
		{
			for (int i = 0; i < dim; i++)
			{
				unsigned int
					x = i % w,
					y = i / w;

				double colorsCaptured = cumulativeFrameCount;

				sf::Color sfPix = renderImg.getPixel({ x, y });
				Color pix = sfPix;

				if (cumulativeLighting)
				{
					render[i] = render[i] + pix;
				}
				else
				{
					render[i] = pix;
					colorsCaptured = 1.0;
				}

				Color displayCol = render[i] / colorsCaptured;

				displayImg.setPixel({ x, y }, {
					(uint8_t)(displayCol.r * 255.0),
					(uint8_t)(displayCol.g * 255.0),
					(uint8_t)(displayCol.b * 255.0)
					});
			}
		}

		{
			shader.setUniform("rndSeed", keepConstant ? 0 : (int)((long)utils::VeryRand(h * w, 4294967295u) - 2147483647));

			float
				viewHeight = tanf((cam.fov / 2.0f) * (float)utils::PI / 180.0f) * 2.0f,
				viewWidth = viewHeight / ((float)h / (float)w);

			shader.setUniform("viewHeight", viewHeight);
			shader.setUniform("viewWidth", viewWidth);

			shader.setUniform("camPos", cam.origin.ToShader());
			shader.setUniform("camFwd", cam.fwd.ToShader());
			shader.setUniform("camUp", cam.up.ToShader());
			shader.setUniform("camRight", cam.right.ToShader());

			shader.setUniform("viewBounds", viewBounds);
			shader.setUniform("realRender", realRender);
			shader.setUniform("disableLighting", disableLighting);
			shader.setUniform("randomizeDir", randomizeSampleDir);

			shader.setUniform("frameCount", cumulativeLighting ? (int)cumulativeFrameCount : 0);
		}

		if (!tex.loadFromImage(renderImg))
			std::cout << "Texture Load Failed!" << std::endl;

		shader.setUniform("lastFrame", renderTex.getTexture());

		renderTex.draw(sprite, &shader);
		renderTex.display();

		if (realRender)
		{
			renderImg = renderTex.getTexture().copyToImage();

			if (!displayTex.loadFromImage(displayImg))
				std::cout << "Texture Load Failed!" << std::endl;
			displaySprite.setTexture(displayTex);
		}
		else
		{
			displaySprite.setTexture(renderTex.getTexture());
		}

		ImGui::End();

		window.clear();
		window.draw(displaySprite);
		ImGui::SFML::Render(window);
		window.display();

		cumulativeFrameCount++;
		totFrames++;
	}

	for (Shape* shape : shapes)
		delete shape;

	delete[] render;
	ImGui::SFML::Shutdown();
	return 0;
}
