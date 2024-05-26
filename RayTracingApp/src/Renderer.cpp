#include "Renderer.h"
#include "RayTracing/Random.h"

namespace Utils
{
	uint32_t ConvertRGBA(glm::vec4 color)
	{
		uint8_t r = (uint8_t)( color.r * 255);
		uint8_t g = (uint8_t)( color.g * 255);
		uint8_t b = (uint8_t)( color.b * 255);
		uint8_t a = (uint8_t)( color.a * 255);
		return (a << 24) | (b << 16) | (g << 8) | r;
	}
};

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<RayTracing::Image>(width, height, RayTracing::ImageFormat::RGBA);
	}
	
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& cam)
{
	const glm::vec3& rayOrigin = cam.GetPosition();

	Ray ray;
	ray.Origin = rayOrigin;

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); ++y)
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); ++x)
		{
			const glm::vec3& rayDirection = cam.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			ray.Direction = rayDirection;

			auto color = TraceRay(scene, ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x+y*m_FinalImage->GetWidth()] = Utils::ConvertRGBA(color);
		}
	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{

	if (scene.Spheres.size() == 0)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();
	for (const Sphere& sphere: scene.Spheres)
	{
		// x = ax+bx*t
		// y = ay+by*t
		// x^2 + y^2 <= r^2

		glm::vec3 origin = ray.Origin - sphere.Postion;
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminate = b * b - 4 * a * c;
		if (discriminate < 0.0f)
			continue;
		float closeT = (-b - glm::sqrt(discriminate)) / (2.0f * a);
		if (closeT < hitDistance)
		{
			hitDistance = closeT;
			closestSphere = &sphere;
		}
	}
	
	if (closestSphere == nullptr)
		return  glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec3 origin = ray.Origin - closestSphere->Postion;
	glm::vec3 hit = origin + ray.Direction * hitDistance;

	glm::vec3 normal = glm::normalize(hit);


	/*hit.x = std::max(0.0f, hit.x);
	hit.y = std::max(0.0f, hit.y);
	hit.z = std::max(0.0f, hit.z);*/

	glm::vec3 lightDir(-1, -1, -1);
	lightDir = glm::normalize(lightDir);


	glm::vec3 shpereColor = closestSphere->Albedo;
	float d = glm::max(glm::dot(normal, -lightDir), 0.0f);

	return glm::vec4(d* shpereColor, 1.0f);
	
}
