#include "Utils.h"
#include "Vec3.h"
#include "Graphics.h"
#include "Shapes.h"
#include "ShaderUniformTypes.h"

#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui-SFML.h"
#include "ImGuiUtils.h"

#include "document.h"
#include "writer.h"
#include "prettywriter.h"
#include "stringbuffer.h"
#include "filereadstream.h"
#include "filewritestream.h"

#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <cmath>


static void SaveScene(const std::vector<Shape*>& shapes, const RaytracerData& data, const std::string& saveName)
{
	rapidjson::Document doc;
	doc.SetObject();

	rapidjson::Value camObj(rapidjson::kObjectType);
	{
		camObj.AddMember("fov", data.cam.fov, doc.GetAllocator());
		camObj.AddMember("perspective", data.cam.perspective, doc.GetAllocator());
		camObj.AddMember("speed", data.cam.speed, doc.GetAllocator());
		camObj.AddMember("origin", rapidjson::Value().SetArray().PushBack(data.cam.origin.x, doc.GetAllocator()).PushBack(data.cam.origin.y, doc.GetAllocator()).PushBack(data.cam.origin.z, doc.GetAllocator()), doc.GetAllocator());
		camObj.AddMember("fwd", rapidjson::Value().SetArray().PushBack(data.cam.fwd.x, doc.GetAllocator()).PushBack(data.cam.fwd.y, doc.GetAllocator()).PushBack(data.cam.fwd.z, doc.GetAllocator()), doc.GetAllocator());
		camObj.AddMember("right", rapidjson::Value().SetArray().PushBack(data.cam.right.x, doc.GetAllocator()).PushBack(data.cam.right.y, doc.GetAllocator()).PushBack(data.cam.right.z, doc.GetAllocator()), doc.GetAllocator());
		camObj.AddMember("up", rapidjson::Value().SetArray().PushBack(data.cam.up.x, doc.GetAllocator()).PushBack(data.cam.up.y, doc.GetAllocator()).PushBack(data.cam.up.z, doc.GetAllocator()), doc.GetAllocator());
	}
	doc.AddMember("cam", camObj, doc.GetAllocator());

	rapidjson::Value skyboxObj(rapidjson::kObjectType);
	{
		skyboxObj.AddMember("showSkybox", data.skybox.showSkybox, doc.GetAllocator());
		skyboxObj.AddMember("peakCol", rapidjson::Value().SetArray().PushBack(data.skybox.peakCol.x, doc.GetAllocator()).PushBack(data.skybox.peakCol.y, doc.GetAllocator()).PushBack(data.skybox.peakCol.z, doc.GetAllocator()), doc.GetAllocator());
		skyboxObj.AddMember("horizonCol", rapidjson::Value().SetArray().PushBack(data.skybox.horizonCol.x, doc.GetAllocator()).PushBack(data.skybox.horizonCol.y, doc.GetAllocator()).PushBack(data.skybox.horizonCol.z, doc.GetAllocator()), doc.GetAllocator());
		skyboxObj.AddMember("voidCol", rapidjson::Value().SetArray().PushBack(data.skybox.voidCol.x, doc.GetAllocator()).PushBack(data.skybox.voidCol.y, doc.GetAllocator()).PushBack(data.skybox.voidCol.z, doc.GetAllocator()), doc.GetAllocator());
		skyboxObj.AddMember("sunCol", rapidjson::Value().SetArray().PushBack(data.skybox.sunCol.x, doc.GetAllocator()).PushBack(data.skybox.sunCol.y, doc.GetAllocator()).PushBack(data.skybox.sunCol.z, doc.GetAllocator()), doc.GetAllocator());
		skyboxObj.AddMember("sunDir", rapidjson::Value().SetArray().PushBack(data.skybox.sunDir.x, doc.GetAllocator()).PushBack(data.skybox.sunDir.y, doc.GetAllocator()).PushBack(data.skybox.sunDir.z, doc.GetAllocator()), doc.GetAllocator());
		skyboxObj.AddMember("sunSize", data.skybox.sunSize, doc.GetAllocator());
		skyboxObj.AddMember("sunFlare", data.skybox.sunFlare, doc.GetAllocator());
	}
	doc.AddMember("skybox", skyboxObj, doc.GetAllocator());

	rapidjson::Value shapesArr(rapidjson::kArrayType);

	for (const auto* shape : shapes)
	{
		rapidjson::Value shapeObj(rapidjson::kObjectType);
		shapeObj.AddMember("type", rapidjson::Value(shape->Name(), doc.GetAllocator()), doc.GetAllocator());

		// Add material properties
		shapeObj.AddMember("albedo", rapidjson::Value().SetArray().PushBack(shape->mat.albedo.x, doc.GetAllocator()).PushBack(shape->mat.albedo.y, doc.GetAllocator()).PushBack(shape->mat.albedo.z, doc.GetAllocator()).PushBack(shape->mat.albedo.w, doc.GetAllocator()), doc.GetAllocator());
		shapeObj.AddMember("specular", rapidjson::Value().SetArray().PushBack(shape->mat.specular.x, doc.GetAllocator()).PushBack(shape->mat.specular.y, doc.GetAllocator()).PushBack(shape->mat.specular.z, doc.GetAllocator()).PushBack(shape->mat.specular.w, doc.GetAllocator()), doc.GetAllocator());
		shapeObj.AddMember("emission", rapidjson::Value().SetArray().PushBack(shape->mat.emission.x, doc.GetAllocator()).PushBack(shape->mat.emission.y, doc.GetAllocator()).PushBack(shape->mat.emission.z, doc.GetAllocator()).PushBack(shape->mat.emission.w, doc.GetAllocator()), doc.GetAllocator());
		shapeObj.AddMember("absorption", rapidjson::Value().SetArray().PushBack(shape->mat.absorption.x, doc.GetAllocator()).PushBack(shape->mat.absorption.y, doc.GetAllocator()).PushBack(shape->mat.absorption.z, doc.GetAllocator()).PushBack(shape->mat.absorption.w, doc.GetAllocator()), doc.GetAllocator());
		shapeObj.AddMember("albedoReflectivity", shape->mat.albedoReflectivity, doc.GetAllocator());
		shapeObj.AddMember("specularReflectivity", shape->mat.specularReflectivity, doc.GetAllocator());
		shapeObj.AddMember("reflectiveIndex", shape->mat.reflectiveIndex, doc.GetAllocator());

		if (auto* aabb = dynamic_cast<const ShapeAABB*>(shape))
		{
			shapeObj.AddMember("min", rapidjson::Value().SetArray().PushBack(aabb->min.x, doc.GetAllocator()).PushBack(aabb->min.y, doc.GetAllocator()).PushBack(aabb->min.z, doc.GetAllocator()), doc.GetAllocator());
			shapeObj.AddMember("max", rapidjson::Value().SetArray().PushBack(aabb->max.x, doc.GetAllocator()).PushBack(aabb->max.y, doc.GetAllocator()).PushBack(aabb->max.z, doc.GetAllocator()), doc.GetAllocator());
		}
		else if (auto* obb = dynamic_cast<const ShapeOBB*>(shape))
		{
			shapeObj.AddMember("center", rapidjson::Value().SetArray().PushBack(obb->center.x, doc.GetAllocator()).PushBack(obb->center.y, doc.GetAllocator()).PushBack(obb->center.z, doc.GetAllocator()), doc.GetAllocator());
			shapeObj.AddMember("halfLength", rapidjson::Value().SetArray().PushBack(obb->halfLength.x, doc.GetAllocator()).PushBack(obb->halfLength.y, doc.GetAllocator()).PushBack(obb->halfLength.z, doc.GetAllocator()), doc.GetAllocator());
			shapeObj.AddMember("xAxis", rapidjson::Value().SetArray().PushBack(obb->xAxis.x, doc.GetAllocator()).PushBack(obb->xAxis.y, doc.GetAllocator()).PushBack(obb->xAxis.z, doc.GetAllocator()), doc.GetAllocator());
			shapeObj.AddMember("yAxis", rapidjson::Value().SetArray().PushBack(obb->yAxis.x, doc.GetAllocator()).PushBack(obb->yAxis.y, doc.GetAllocator()).PushBack(obb->yAxis.z, doc.GetAllocator()), doc.GetAllocator());
			shapeObj.AddMember("zAxis", rapidjson::Value().SetArray().PushBack(obb->zAxis.x, doc.GetAllocator()).PushBack(obb->zAxis.y, doc.GetAllocator()).PushBack(obb->zAxis.z, doc.GetAllocator()), doc.GetAllocator());
		}
		else if (auto* sphere = dynamic_cast<const ShapeSphere*>(shape))
		{
			shapeObj.AddMember("pos", rapidjson::Value().SetArray().PushBack(sphere->pos.x, doc.GetAllocator()).PushBack(sphere->pos.y, doc.GetAllocator()).PushBack(sphere->pos.z, doc.GetAllocator()), doc.GetAllocator());
			shapeObj.AddMember("rad", sphere->rad, doc.GetAllocator());
		}
		else if (auto* tri = dynamic_cast<const ShapeTri*>(shape))
		{
			shapeObj.AddMember("v1", rapidjson::Value().SetArray().PushBack(tri->v1.x, doc.GetAllocator()).PushBack(tri->v1.y, doc.GetAllocator()).PushBack(tri->v1.z, doc.GetAllocator()), doc.GetAllocator());
			shapeObj.AddMember("v2", rapidjson::Value().SetArray().PushBack(tri->v2.x, doc.GetAllocator()).PushBack(tri->v2.y, doc.GetAllocator()).PushBack(tri->v2.z, doc.GetAllocator()), doc.GetAllocator());
			shapeObj.AddMember("v3", rapidjson::Value().SetArray().PushBack(tri->v3.x, doc.GetAllocator()).PushBack(tri->v3.y, doc.GetAllocator()).PushBack(tri->v3.z, doc.GetAllocator()), doc.GetAllocator());
		}
		else if (auto* plane = dynamic_cast<const ShapePlane*>(shape))
		{
			shapeObj.AddMember("center", rapidjson::Value().SetArray().PushBack(plane->center.x, doc.GetAllocator()).PushBack(plane->center.y, doc.GetAllocator()).PushBack(plane->center.z, doc.GetAllocator()), doc.GetAllocator());
			shapeObj.AddMember("normal", rapidjson::Value().SetArray().PushBack(plane->normal.x, doc.GetAllocator()).PushBack(plane->normal.y, doc.GetAllocator()).PushBack(plane->normal.z, doc.GetAllocator()), doc.GetAllocator());
		}

		shapesArr.PushBack(shapeObj, doc.GetAllocator());
	}

	doc.AddMember("shapes", shapesArr, doc.GetAllocator());

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	std::ofstream ofs("Scenes/" + saveName + ".json");
	if (ofs.is_open())
	{
		ofs << buffer.GetString();
		ofs.close();
	}
	else
	{
		std::cerr << "Failed to save scene to file: " << saveName << std::endl;
	}
}

static std::vector<Shape*> LoadScene(const std::string& fileName, std::vector<Shape*> &shapes, RaytracerData& data)
{
	shapes.clear();

	std::ifstream ifs("Scenes/" + fileName + ".json");
	if (!ifs.is_open())
	{
		std::cerr << "Failed to open scene file: " << fileName << std::endl;
		return shapes;
	}

	std::string jsonStr((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	ifs.close();

	rapidjson::Document doc;
	if (doc.Parse(jsonStr.c_str()).HasParseError())
	{
		std::cerr << "Failed to parse scene JSON: " << rapidjson::GetParseErrorFunc(doc.GetParseError()) << std::endl;
		return shapes;
	}

	if (doc.HasMember("cam") && doc["cam"].IsObject())
	{
		const auto& camObj = doc["cam"];
		if (camObj.HasMember("fov") && camObj["fov"].IsFloat())
			data.cam.fov = camObj["fov"].GetFloat();
		if (camObj.HasMember("perspective") && camObj["perspective"].IsBool())
			data.cam.perspective = camObj["perspective"].GetBool();
		if (camObj.HasMember("speed") && camObj["speed"].IsFloat())
			data.cam.speed = camObj["speed"].GetFloat();
		if (camObj.HasMember("origin") && camObj["origin"].IsArray() && camObj["origin"].Size() == 3)
			data.cam.origin = Vec3(camObj["origin"][0].GetFloat(), camObj["origin"][1].GetFloat(), camObj["origin"][2].GetFloat());
		if (camObj.HasMember("fwd") && camObj["fwd"].IsArray() && camObj["fwd"].Size() == 3)
			data.cam.fwd = Vec3(camObj["fwd"][0].GetFloat(), camObj["fwd"][1].GetFloat(), camObj["fwd"][2].GetFloat());
		if (camObj.HasMember("right") && camObj["right"].IsArray() && camObj["right"].Size() == 3)
			data.cam.right = Vec3(camObj["right"][0].GetFloat(), camObj["right"][1].GetFloat(), camObj["right"][2].GetFloat());
		if (camObj.HasMember("up") && camObj["up"].IsArray() && camObj["up"].Size() == 3)
			data.cam.up = Vec3(camObj["up"][0].GetFloat(), camObj["up"][1].GetFloat(), camObj["up"][2].GetFloat());
		data.cam.UpdateRotation();
	}
	else
	{
		Viewport prevViewport = data.cam.viewport; // Preserve viewport settings
		data.cam = Cam(); // Reset to default if camera data is missing
		data.cam.viewport = prevViewport; // Restore viewport settings
	}

	if (doc.HasMember("skybox") && doc["skybox"].IsObject())
	{
		const auto& skyboxObj = doc["skybox"];

		if (skyboxObj.HasMember("showSkybox") && skyboxObj["showSkybox"].IsBool())
			data.skybox.showSkybox = skyboxObj["showSkybox"].GetBool();
		if (skyboxObj.HasMember("peakCol") && skyboxObj["peakCol"].IsArray() && skyboxObj["peakCol"].Size() == 3)
			data.skybox.peakCol = sf::Glsl::Vec3(skyboxObj["peakCol"][0].GetFloat(), skyboxObj["peakCol"][1].GetFloat(), skyboxObj["peakCol"][2].GetFloat());
		if (skyboxObj.HasMember("horizonCol") && skyboxObj["horizonCol"].IsArray() && skyboxObj["horizonCol"].Size() == 3)
			data.skybox.horizonCol = sf::Glsl::Vec3(skyboxObj["horizonCol"][0].GetFloat(), skyboxObj["horizonCol"][1].GetFloat(), skyboxObj["horizonCol"][2].GetFloat());
		if (skyboxObj.HasMember("voidCol") && skyboxObj["voidCol"].IsArray() && skyboxObj["voidCol"].Size() == 3)
			data.skybox.voidCol = sf::Glsl::Vec3(skyboxObj["voidCol"][0].GetFloat(), skyboxObj["voidCol"][1].GetFloat(), skyboxObj["voidCol"][2].GetFloat());
		if (skyboxObj.HasMember("sunCol") && skyboxObj["sunCol"].IsArray() && skyboxObj["sunCol"].Size() == 3)
			data.skybox.sunCol = sf::Glsl::Vec3(skyboxObj["sunCol"][0].GetFloat(), skyboxObj["sunCol"][1].GetFloat(), skyboxObj["sunCol"][2].GetFloat());
		if (skyboxObj.HasMember("sunDir") && skyboxObj["sunDir"].IsArray() && skyboxObj["sunDir"].Size() == 3)
			data.skybox.sunDir = sf::Glsl::Vec3(skyboxObj["sunDir"][0].GetFloat(), skyboxObj["sunDir"][1].GetFloat(), skyboxObj["sunDir"][2].GetFloat());
		if (skyboxObj.HasMember("sunSize") && skyboxObj["sunSize"].IsFloat())
			data.skybox.sunSize = skyboxObj["sunSize"].GetFloat();
		if (skyboxObj.HasMember("sunFlare") && skyboxObj["sunFlare"].IsFloat())
			data.skybox.sunFlare = skyboxObj["sunFlare"].GetFloat();
	}
	else
	{
		data.skybox = Skybox(); // Reset to default if skybox data is missing
	}


	if (!doc.HasMember("shapes") || !doc["shapes"].IsArray())
	{
		std::cerr << "Invalid scene format: 'shapes' array not found" << std::endl;
		return shapes;
	}

	const auto& shapesArr = doc["shapes"].GetArray();
	for (const auto& shapeVal : shapesArr)
	{
		if (!shapeVal.IsObject() || !shapeVal.HasMember("type") || !shapeVal["type"].IsString())
			continue;

		Shape* shape = nullptr;
		std::string type = shapeVal["type"].GetString();

		if (type == "aabb")
		{
			ShapeAABB *aabb = new ShapeAABB();

			if (shapeVal.HasMember("min") && shapeVal["min"].IsArray() && shapeVal["min"].Size() == 3)
				aabb->min = sf::Glsl::Vec3(shapeVal["min"][0].GetFloat(), shapeVal["min"][1].GetFloat(), shapeVal["min"][2].GetFloat());
			if (shapeVal.HasMember("max") && shapeVal["max"].IsArray() && shapeVal["max"].Size() == 3)
				aabb->max = sf::Glsl::Vec3(shapeVal["max"][0].GetFloat(), shapeVal["max"][1].GetFloat(), shapeVal["max"][2].GetFloat());

			shape = aabb;
		}
		else if (type == "obb")
		{
			ShapeOBB *obb = new ShapeOBB();

			if (shapeVal.HasMember("center") && shapeVal["center"].IsArray() && shapeVal["center"].Size() == 3)
				obb->center = sf::Glsl::Vec3(shapeVal["center"][0].GetFloat(), shapeVal["center"][1].GetFloat(), shapeVal["center"][2].GetFloat());
			if (shapeVal.HasMember("halfLength") && shapeVal["halfLength"].IsArray() && shapeVal["halfLength"].Size() == 3)
				obb->halfLength = sf::Glsl::Vec3(shapeVal["halfLength"][0].GetFloat(), shapeVal["halfLength"][1].GetFloat(), shapeVal["halfLength"][2].GetFloat());
			if (shapeVal.HasMember("xAxis") && shapeVal["xAxis"].IsArray() && shapeVal["xAxis"].Size() == 3)
				obb->xAxis = sf::Glsl::Vec3(shapeVal["xAxis"][0].GetFloat(), shapeVal["xAxis"][1].GetFloat(), shapeVal["xAxis"][2].GetFloat());
			if (shapeVal.HasMember("yAxis") && shapeVal["yAxis"].IsArray() && shapeVal["yAxis"].Size() == 3)
				obb->yAxis = sf::Glsl::Vec3(shapeVal["yAxis"][0].GetFloat(), shapeVal["yAxis"][1].GetFloat(), shapeVal["yAxis"][2].GetFloat());
			if (shapeVal.HasMember("zAxis") && shapeVal["zAxis"].IsArray() && shapeVal["zAxis"].Size() == 3)
				obb->zAxis = sf::Glsl::Vec3(shapeVal["zAxis"][0].GetFloat(), shapeVal["zAxis"][1].GetFloat(), shapeVal["zAxis"][2].GetFloat());

			shape = obb;
		}
		else if (type == "sphere")
		{
			ShapeSphere *sphere = new ShapeSphere();

			if (shapeVal.HasMember("pos") && shapeVal["pos"].IsArray() && shapeVal["pos"].Size() == 3)
				sphere->pos = sf::Glsl::Vec3(shapeVal["pos"][0].GetFloat(), shapeVal["pos"][1].GetFloat(), shapeVal["pos"][2].GetFloat());
			if (shapeVal.HasMember("rad") && shapeVal["rad"].IsFloat())
				sphere->rad = shapeVal["rad"].GetFloat();

			shape = sphere;
		}
		else if (type == "tri")
		{
			ShapeTri *tri = new ShapeTri();

			if (shapeVal.HasMember("v1") && shapeVal["v1"].IsArray() && shapeVal["v1"].Size() == 3)
				tri->v1 = sf::Glsl::Vec3(shapeVal["v1"][0].GetFloat(), shapeVal["v1"][1].GetFloat(), shapeVal["v1"][2].GetFloat());
			if (shapeVal.HasMember("v2") && shapeVal["v2"].IsArray() && shapeVal["v2"].Size() == 3)
				tri->v2 = sf::Glsl::Vec3(shapeVal["v2"][0].GetFloat(), shapeVal["v2"][1].GetFloat(), shapeVal["v2"][2].GetFloat());
			if (shapeVal.HasMember("v3") && shapeVal["v3"].IsArray() && shapeVal["v3"].Size() == 3)
				tri->v3 = sf::Glsl::Vec3(shapeVal["v3"][0].GetFloat(), shapeVal["v3"][1].GetFloat(), shapeVal["v3"][2].GetFloat());

			shape = tri;
		}
		else if (type == "plane")
		{
			ShapePlane *plane = new ShapePlane();

			if (shapeVal.HasMember("center") && shapeVal["center"].IsArray() && shapeVal["center"].Size() == 3)
				plane->center = sf::Glsl::Vec3(shapeVal["center"][0].GetFloat(), shapeVal["center"][1].GetFloat(), shapeVal["center"][2].GetFloat());
			if (shapeVal.HasMember("normal") && shapeVal["normal"].IsArray() && shapeVal["normal"].Size() == 3)
				plane->normal = sf::Glsl::Vec3(shapeVal["normal"][0].GetFloat(), shapeVal["normal"][1].GetFloat(), shapeVal["normal"][2].GetFloat());

			shape = plane;
		}

		if (shape)
		{
			if (shapeVal.HasMember("albedo") && shapeVal["albedo"].IsArray() && shapeVal["albedo"].Size() == 4)
				shape->mat.albedo = sf::Glsl::Vec4(shapeVal["albedo"][0].GetFloat(), shapeVal["albedo"][1].GetFloat(), shapeVal["albedo"][2].GetFloat(), shapeVal["albedo"][3].GetFloat());
			if (shapeVal.HasMember("specular") && shapeVal["specular"].IsArray() && shapeVal["specular"].Size() == 4)
				shape->mat.specular = sf::Glsl::Vec4(shapeVal["specular"][0].GetFloat(), shapeVal["specular"][1].GetFloat(), shapeVal["specular"][2].GetFloat(), shapeVal["specular"][3].GetFloat());
			if (shapeVal.HasMember("emission") && shapeVal["emission"].IsArray() && shapeVal["emission"].Size() == 4)
				shape->mat.emission = sf::Glsl::Vec4(shapeVal["emission"][0].GetFloat(), shapeVal["emission"][1].GetFloat(), shapeVal["emission"][2].GetFloat(), shapeVal["emission"][3].GetFloat());
			if (shapeVal.HasMember("absorption") && shapeVal["absorption"].IsArray() && shapeVal["absorption"].Size() == 4)
				shape->mat.absorption = sf::Glsl::Vec4(shapeVal["absorption"][0].GetFloat(), shapeVal["absorption"][1].GetFloat(), shapeVal["absorption"][2].GetFloat(), shapeVal["absorption"][3].GetFloat());
			if (shapeVal.HasMember("albedoReflectivity") && shapeVal["albedoReflectivity"].IsFloat())
				shape->mat.albedoReflectivity = shapeVal["albedoReflectivity"].GetFloat();
			if (shapeVal.HasMember("specularReflectivity") && shapeVal["specularReflectivity"].IsFloat())
				shape->mat.specularReflectivity = shapeVal["specularReflectivity"].GetFloat();
			if (shapeVal.HasMember("reflectiveIndex") && shapeVal["reflectiveIndex"].IsFloat())
				shape->mat.reflectiveIndex = shapeVal["reflectiveIndex"].GetFloat();
		
			shapes.push_back(shape);
		}
	}

	return shapes;
}

static std::vector<std::string> GetSceneList()
{
	std::vector<std::string> sceneNames;

	for (const auto& entry : std::filesystem::directory_iterator("Scenes"))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".json")
		{
			sceneNames.push_back(entry.path().stem().string());
		}
	}

	return sceneNames;
}


static bool EditMaterial(Material &mat)
{
	bool isEdited = false;

	if (ImGui::TreeNode("Material"))
	{
		ImGuiColorEditFlags flags = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float;

		isEdited |= ImGui::ColorEdit4("Albedo", &mat.albedo.x, flags);
		ImGuiUtils::LockMouseOnActive();

		isEdited |= ImGui::ColorEdit4("Specular", &mat.specular.x, flags);
		ImGuiUtils::LockMouseOnActive();

		isEdited |= ImGui::ColorEdit4("Emission", &mat.emission.x, flags);
		ImGuiUtils::LockMouseOnActive();

		isEdited |= ImGui::ColorEdit4("Absorption", &mat.absorption.x, flags);
		ImGuiUtils::LockMouseOnActive();

		isEdited |= ImGui::SliderFloat("Albedo Reflectivity", &mat.albedoReflectivity, 0.0f, 1.0f);
		isEdited |= ImGui::SliderFloat("Specular Reflectivity", &mat.specularReflectivity, 0.0f, 1.0f);

		isEdited |= ImGui::DragFloat("Reflective Index", &mat.reflectiveIndex, 0.001f, 0.001f);
		ImGuiUtils::LockMouseOnActive();

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
			ImGuiUtils::LockMouseOnActive();

			isEdited |= ImGui::DragFloat3("Max", &aabb->max.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();
		}
	}
	else if (auto* obb = dynamic_cast<ShapeOBB*>(&shape))
	{
		isOpen = ImGui::TreeNode("OBB");

		if (isOpen)
		{
			isEdited |= ImGui::DragFloat3("Center", &obb->center.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			isEdited |= ImGui::DragFloat3("Half Extents", &obb->halfLength.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			isEdited |= ImGui::DragFloat3("X-Axis", &obb->xAxis.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			isEdited |= ImGui::DragFloat3("Y-Axis", &obb->yAxis.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			isEdited |= ImGui::DragFloat3("Z-Axis", &obb->zAxis.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			if (ImGui::Button("Normalize"))
			{
				obb->xAxis = obb->xAxis.normalized();
				obb->yAxis = (obb->yAxis - obb->xAxis * obb->yAxis.dot(obb->xAxis)).normalized();
				obb->zAxis = (obb->zAxis - obb->xAxis * obb->zAxis.dot(obb->xAxis) - obb->yAxis * obb->zAxis.dot(obb->yAxis)).normalized();
				isEdited = true;
			}
		}
	}
	else if (auto* sphere = dynamic_cast<ShapeSphere*>(&shape))
	{
		isOpen = ImGui::TreeNode("Sphere");

		if (isOpen)
		{
			isEdited |= ImGui::DragFloat3("Position", &sphere->pos.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			isEdited |= ImGui::DragFloat("Radius", &sphere->rad, 0.1f);
			ImGuiUtils::LockMouseOnActive();
		}
	}
	else if (auto* tri = dynamic_cast<ShapeTri*>(&shape))
	{
		isOpen = ImGui::TreeNode("Triangle");

		if (isOpen)
		{
			isEdited |= ImGui::DragFloat3("Vertex 1", &tri->v1.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			isEdited |= ImGui::DragFloat3("Vertex 2", &tri->v2.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			isEdited |= ImGui::DragFloat3("Vertex 3", &tri->v3.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();
		}
	}
	else if (auto* plane = dynamic_cast<ShapePlane*>(&shape))
	{
		isOpen = ImGui::TreeNode("Plane");

		if (isOpen)
		{
			isEdited |= ImGui::DragFloat3("Center", &plane->center.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			isEdited |= ImGui::DragFloat3("Normal", &plane->normal.x, 0.1f);
			ImGuiUtils::LockMouseOnActive();

			if (ImGui::Button("Normalize"))
			{
				plane->normal = plane->normal.normalized();
				isEdited = true;
			}
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


	unsigned int nextSnapshot = 0;

	// Build Scene
	RaytracerData rtData;

	rtData.cam = Cam(
		75.0f, true, 5.0f,
		Vec3(0.0, 5.0, -10.0),
		Vec3(0.0, -0.531709431, 1.0).Normalize(),
		Viewport()
	);

	sf::RenderWindow window(sf::VideoMode(rtData.cam.viewport.ToVecU()), "SFML Window", sf::State::Windowed);
	if (!ImGui::SFML::Init(window))
		std::cout << "Failed to initialize ImGui-SFML" << std::endl;

	Color* render = new Color[rtData.cam.viewport.dim];
	for (int i = 0; i < rtData.cam.viewport.dim; i++)
		render[i] = Color();

	sf::Image
		renderImg(rtData.cam.viewport.ToVecU(), sf::Color::Black),
		displayImg(rtData.cam.viewport.ToVecU(), sf::Color::Black);

	sf::RenderTexture renderTex(rtData.cam.viewport.ToVecU());

	sf::Texture tex(rtData.cam.viewport.ToVecU()), displayTex(rtData.cam.viewport.ToVecU());

	sf::Sprite sprite(tex), displaySprite(displayTex);

	std::vector<Shape*> shapes; 
	LoadScene("Scene 1", shapes, rtData);

	sf::Vector2i deltas, windowPos;

	bool cumulativeLighting, realRender, randomizeSampleDir, keepConstant, giveControl, disableLighting, viewBounds;
	unsigned int perPixelSamples, maxBounces;

	{
		keepConstant = false;
		giveControl = false;

		cumulativeLighting = true;
		realRender = false;
		randomizeSampleDir = true;
		disableLighting = false;
		viewBounds = false;
		perPixelSamples = 8;
		maxBounces = 6;
	}


	sf::Shader shader;
	if (!shader.loadFromFile("RaytracerShader.frag", sf::Shader::Type::Fragment))
	{
		std::cerr << "Failed to load shader" << std::endl;
		return -1;
	}

	rtData.BindAll(shader);

	shader.setUniform("samples", (int)perPixelSamples);
	shader.setUniform("maxBounces", (int)maxBounces);

	BindShapes(shapes, shader);


	sf::Clock clock, imClock;
	double lT = 0.0, tT = 0.0, dT = 0.0;

	bool lockFPS = false;

	unsigned int
		cumulativeFrameCount = 0,
		totFrames = 0,
		maxFPS = 60;

	double timeToSleep = 0.0;

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
					float lFov = rtData.cam.fov;
					rtData.cam.fov = std::clamp(rtData.cam.fov - eventSubtype->delta, 0.01f, 179.99f);

					if (abs(rtData.cam.fov - lFov) > 0.000001)
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

				rtData.cam.viewport = Viewport(eventSubtype->size.x, eventSubtype->size.y);
				
				window.setView(sf::View(rtData.cam.viewport.ToVecF().componentWiseMul({0.5f, 0.5f}), rtData.cam.viewport.ToVecF()));

				delete[] render;
				render = new Color[rtData.cam.viewport.dim];
				for (int i = 0; i < rtData.cam.viewport.dim; i++)
					render[i] = Color();

				renderImg = sf::Image(rtData.cam.viewport.ToVecU(), sf::Color::Black);
				displayImg = sf::Image(rtData.cam.viewport.ToVecU(), sf::Color::Black);

				renderTex = sf::RenderTexture(rtData.cam.viewport.ToVecU());

				tex = sf::Texture(rtData.cam.viewport.ToVecU());
				displayTex = sf::Texture(rtData.cam.viewport.ToVecU());

				sprite = sf::Sprite(tex);
				displaySprite = sf::Sprite(displayTex);

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

		if (ImGui::BeginTabBar("Tabs"))
		{
			if (ImGui::BeginTabItem("Scene"))
			{
				bool isEdited = false;

				ImGui::SeparatorText("Saving/Loading");

				static std::string saveName = "Scene 1";

				if (ImGui::Button("Save"))
					SaveScene(shapes, rtData, saveName);

				ImGui::SameLine();
				ImGui::InputText("##SaveName", &saveName);

				std::vector<std::string> sceneList = GetSceneList();
				static int currSceneIndex = -1;


				if (ImGui::Button("Load"))
				{
					if (currSceneIndex >= 0 && currSceneIndex < sceneList.size())
					{
						for (Shape* shape : shapes)
							delete shape;

						LoadScene(sceneList[currSceneIndex], shapes, rtData);
						isEdited = true;
					}
				}

				auto comboFunc = [](void* data, int idx, const char** out_text){
					const std::vector<std::string>* scenes = static_cast<std::vector<std::string>*>(data);
					if (idx < 0 || idx >= scenes->size())
						return false;
					*out_text = (*scenes)[idx].c_str();
					return true;
				};

				ImGui::SameLine();
				ImGui::Combo("##SceneList", &currSceneIndex, comboFunc, &sceneList, sceneList.size());


				ImGui::SeparatorText("Scene Editing");

				if (ImGui::TreeNode("Skybox"))
				{
					isEdited |= ImGui::Checkbox("Show Skybox", &rtData.skybox.showSkybox);

					ImGuiColorEditFlags flags = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float;
					isEdited |= ImGui::ColorEdit3("Peak Color", &rtData.skybox.peakCol.x, flags);
					ImGuiUtils::LockMouseOnActive();

					isEdited |= ImGui::ColorEdit3("Horizon Color", &rtData.skybox.horizonCol.x, flags);
					ImGuiUtils::LockMouseOnActive();

					isEdited |= ImGui::ColorEdit3("Void Color", &rtData.skybox.voidCol.x, flags);
					ImGuiUtils::LockMouseOnActive();

					isEdited |= ImGui::ColorEdit3("Sun Color", &rtData.skybox.sunCol.x, flags);
					ImGuiUtils::LockMouseOnActive();

					isEdited |= ImGui::DragFloat3("Sun Direction", &rtData.skybox.sunDir.x, 0.1f);
					ImGuiUtils::LockMouseOnActive();

					isEdited |= ImGui::DragFloat("Sun Size", &rtData.skybox.sunSize, 0.1f, 0.0f);
					ImGuiUtils::LockMouseOnActive();

					isEdited |= ImGui::DragFloat("Sun Flare", &rtData.skybox.sunFlare, 0.1f, 0.0f);
					ImGuiUtils::LockMouseOnActive();

					ImGui::TreePop();
				}

				// Shape Type Selection and Addition
				enum class ShapeType { AABB, OBB, Sphere, Tri, Plane };
				static ShapeType currSelectedShape = ShapeType::AABB;

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

				ImGui::SameLine();
				ImGui::Combo("##ShapeType", (int*)&currSelectedShape, "AABB\0OBB\0Sphere\0Triangle\0Plane\0\0");


				// Shape List and Editing
				ImGui::BeginChild("Shapes", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Borders);
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
					rtData.skybox.Bind(shader);
					BindShapes(shapes, shader);
					hasMoved = true;
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Camera"))
			{
				ImGui::SliderFloat("Speed", &rtData.cam.speed, 0.01f, 100.0f);
				ImGuiUtils::LockMouseOnActive();

				if (ImGui::SliderFloat("FOV", &rtData.cam.fov, 0.01f, 179.99f))
					hasMoved = true;
				ImGuiUtils::LockMouseOnActive();

				if (ImGui::DragScalarN("Position", ImGuiDataType_Double, &rtData.cam.origin.x, 3, 0.1f))
					hasMoved = true;
				ImGuiUtils::LockMouseOnActive();

				if (ImGui::DragScalarN("Forward", ImGuiDataType_Double, &rtData.cam.fwd.x, 3, 0.1f))
				{
					rtData.cam.UpdateRotation();
					hasMoved = true;
				}
				ImGuiUtils::LockMouseOnActive();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Rendering"))
			{
				ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

				if (ImGui::Button("Take Snapshot"))
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

				sf::Vector2u res = rtData.cam.viewport.ToVecU();
				if (ImGui::DragScalarN("Resolution", ImGuiDataType_U32, &res.x, 2, 1.0f))
				{
					res.x = std::max(1u, res.x);
					res.y = std::max(1u, res.y);
					window.setSize(res);
				}
				ImGuiUtils::LockMouseOnActive();

				if (ImGui::Checkbox("Lock FPS", &lockFPS))
					timeToSleep = (lockFPS) ? ((1.0 / (double)maxFPS) - dT) : 0.0;

				if (lockFPS)
				{
					ImGui::SameLine();

					int maxFPSint = (int)maxFPS;
					if (ImGui::DragInt("##MaxFPS", &maxFPSint, 0.5f, 10))
					{
						maxFPS = (unsigned int)std::max(10, maxFPSint);
						timeToSleep = (1.0 / (double)maxFPS) - dT;
					}
					ImGuiUtils::LockMouseOnActive();
				}

				if (ImGui::DragInt("Per Pixel Samples", (int*)&perPixelSamples, 1.0f, 1, 1024))
				{
					shader.setUniform("samples", (int)perPixelSamples);
					cumulativeFrameCount = 0;
					hasMoved = true;
				}
				ImGuiUtils::LockMouseOnActive();

				if (ImGui::DragInt("Max Bounces", (int*)&maxBounces, 1.0f, 1, 64))
				{
					shader.setUniform("maxBounces", (int)maxBounces);
					cumulativeFrameCount = 0;
					hasMoved = true;
				}
				ImGuiUtils::LockMouseOnActive();

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
			double speedMult = (double)rtData.cam.speed * dT * (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ? 3.0 : 1.0);
			speedMult /= (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ? 6.0 : 1.0);

			Vec3 camLOrigin = rtData.cam.origin;
			Vec3 camLFwd = rtData.cam.fwd;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
				rtData.cam.origin += rtData.cam.fwd * speedMult;
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
				rtData.cam.origin -= rtData.cam.fwd * speedMult;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
				rtData.cam.origin += rtData.cam.right * speedMult;
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
				rtData.cam.origin -= rtData.cam.right * speedMult;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
				rtData.cam.origin += rtData.cam.up * speedMult;
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X))
				rtData.cam.origin -= rtData.cam.up * speedMult;

			sf::Vector2i fixedPoint = windowPos + rtData.cam.viewport.ToVecI().componentWiseDiv({ 2, 2 });

			windowPos = window.getPosition();
			deltas = fixedPoint - sf::Mouse::getPosition();
			if (deltas != sf::Vector2i(0, 0))
				sf::Mouse::setPosition(fixedPoint);

			if (deltas.y != 0)
			{
				float sign = (float)deltas.y * -0.001f;
				int verticality = (sign > 0) ? -1 : 1;

				Vec3 offAngle = rtData.cam.fwd - Vec3(0, verticality, 0);
				offAngle.NormalizeApprox();

				rtData.cam.fwd = (
					rtData.cam.fwd * cos(sign) +
					rtData.cam.right.Cross(rtData.cam.fwd) * sin(sign) +
					rtData.cam.right * rtData.cam.right.Dot(rtData.cam.fwd) * (1.0f - cos(sign))
				);

				if (offAngle.Dot(rtData.cam.fwd) <= 0)
					rtData.cam.fwd = Vec3(0, verticality, 0) + offAngle * utils::MINVAL * 100.0;
			}

			if (deltas.x != 0 && abs(rtData.cam.fwd.y) != 1.0)
			{
				float sign = (float)deltas.x * -0.001f;

				rtData.cam.fwd = {
					rtData.cam.fwd.x * cos(sign) + rtData.cam.fwd.z * sin(sign),
					rtData.cam.fwd.y,
					-rtData.cam.fwd.x * sin(sign) + rtData.cam.fwd.z * cos(sign)
				};
			}

			rtData.cam.UpdateRotation();

			if ((camLOrigin - rtData.cam.origin).MagSqr() > 0.000001 || (camLFwd - rtData.cam.fwd).MagSqr() > 0.000001)
				hasMoved = true;
		}

		if (cumulativeLighting && hasMoved)
		{
			cumulativeFrameCount = 0;
			renderTex.clear();

			if (realRender)
			{
				for (int i = 0; i < rtData.cam.viewport.dim; i++)
				{
					render[i] = Color();
					renderImg.setPixel({ i % rtData.cam.viewport.w, i / rtData.cam.viewport.w }, { 0, 0, 0, 0 });
				}
			}
		}

		if (realRender && cumulativeFrameCount > 0)
		{
			for (int i = 0; i < rtData.cam.viewport.dim; i++)
			{
				unsigned int
					x = i % rtData.cam.viewport.w,
					y = i / rtData.cam.viewport.w;

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
			rtData.BindContinuous(shader);

			shader.setUniform("rndSeed", keepConstant ? 0 : (int)((long)utils::VeryRand(rtData.cam.viewport.h * rtData.cam.viewport.w, 4294967295u) - 2147483647));
			
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

		if (!lockFPS)
			continue;

		double minDT = 1.0 / (double)maxFPS;

		if (dT < minDT)
			timeToSleep += (minDT - dT) * 0.9;
		else if (dT > minDT)
			timeToSleep *= 0.98;

		sf::sleep(sf::seconds(timeToSleep));
	}

	for (Shape* shape : shapes)
		delete shape;

	delete[] render;
	ImGui::SFML::Shutdown();
	return 0;
}
