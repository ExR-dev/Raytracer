#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\RaytracerSceneRenderer.h"
#include "Content\RaytracerTextRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace RaytracerDX11
{
	class RaytracerDX11Main : public DX::IDeviceNotify
	{
	public:
		RaytracerDX11Main(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~RaytracerDX11Main();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<RaytracerSceneRenderer> m_sceneRenderer;
		std::unique_ptr<RaytracerTextRenderer> m_fpsTextRenderer;

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}