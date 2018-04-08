#include "BGFX/common.h"
#include "BGFX/bgfx_utils.h"
#include "BGFX/imgui/bgfx_imgui.h"

#include <bx/commandline.h>

#include "Render/AgRenderer.h"
#include "Resource/AgRenderResourceManager.h"
#include "FBX/FbxImportManager.h"
#include "Scene/AgCamera.h"
#include "Scene/AgSceneDatabase.h"
#include "Resource/AgSpotLight.h"

#include <algorithm>
#include <thread>

using namespace ambergris;

namespace
{

class BgFbxViewer : public entry::AppI
{
public:
	BgFbxViewer(const char* _name, const char* _description)
		: entry::AppI(_name, _description)
	{
	}

	static void AddMeshToRenderer(AgMesh* node)
	{
		//Singleton<AgSceneDatabase>::instance().AppendNode(node);
	}

	static bool LoadFBX(const char* filePath)
	{
		if (!filePath)
			return false;

		ambergris_fbx::FbxImportManager fbxManager;

		ambergris_fbx::FbxImportManager::SmtFbxMeshCBFunc meshCB(std::bind(&AddMeshToRenderer, std::placeholders::_1));
		fbxManager.RegisterMeshCBFunc(meshCB);

		Singleton<AgRenderResourceManager>::instance().m_text.appendDynamic(L"Loading FBX...\n");
		if (!fbxManager.Load(filePath))
		{
			Singleton<AgRenderResourceManager>::instance().m_text.appendDynamic(L"Failed to Load FBX\n");
			return false;
		}
		Singleton<AgRenderResourceManager>::instance().m_text.appendDynamic(L"Parsing FBX...\n");
		if (!fbxManager.ParseScene())
		{
			Singleton<AgRenderResourceManager>::instance().m_text.appendDynamic(L"Failed to parse FBX.\n");
			return false;
		}
		Singleton<AgRenderResourceManager>::instance().m_text.appendDynamic(L"Succeed to load and parse FBX.\n");
		return true;
	}

	void focusView(AgCamera* camera)
	{
		if (!camera)
			return;

		const AgSceneDatabase& sceneDB = Singleton<AgSceneDatabase>::instance();

		bool touched = false;
		double bbox_min[3];
		double bbox_max[3];
		bbox_min[0] = std::numeric_limits<double>::max();
		bbox_min[1] = std::numeric_limits<double>::max();
		bbox_min[2] = std::numeric_limits<double>::max();
		bbox_max[0] = std::numeric_limits<double>::min();
		bbox_max[1] = std::numeric_limits<double>::min();
		bbox_max[2] = std::numeric_limits<double>::min();
		const int nNodeNum = (const int)sceneDB.m_objectManager.getSize();
		for (int i = 0; i < nNodeNum; i++)
		{
			const AgMesh* node = dynamic_cast<const AgMesh*>(sceneDB.m_objectManager.get(i));
			if (!node)
				continue;
			const AgBoundingbox* bbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(node->m_bbox);
			if (!bbox || AgBoundingbox::kInvalidHandle == bbox->m_handle)
				continue;

			const AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(node->m_global_transform_h);
			if(!transform)
				continue;

			RealityComputing::Common::RCBox bounds = bbox->m_bounds.getTransformed(transform->m_transform);

			if (bounds.getMin().x < bbox_min[0])
				bbox_min[0] = bounds.getMin().x;
			if (bounds.getMin().y < bbox_min[1])
				bbox_min[1] = bounds.getMin().y;
			if (bounds.getMin().z < bbox_min[2])
				bbox_min[2] = bounds.getMin().z;
			if (bounds.getMax().x > bbox_max[0])
				bbox_max[0] = bounds.getMax().x;
			if (bounds.getMax().y > bbox_max[1])
				bbox_max[1] = bounds.getMax().y;
			if (bounds.getMax().z > bbox_max[2])
				bbox_max[2] = bounds.getMax().z;
			touched = true;
		}
		if (!touched)
			return;

		float scale = std::max((float)(bbox_max[0] - bbox_min[0]), std::max((float)(bbox_max[1] - bbox_min[1]), (float)(bbox_max[2] - bbox_min[2])));
		float target[3] = { (float)(bbox_max[0] + bbox_min[0]) * 0.5f, (float)(bbox_max[1] + bbox_min[1]) * 0.5f, (float)(bbox_max[2] + bbox_min[2]) * 0.5f };
		float eye[3] = { scale, 0.0f, 0.0f };

		camera->setTarget(target);
		camera->setPosition(eye);
		//cameraSetVerticalAngle(-bx::kPi / 4.0f);
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);
		bx::CommandLine cmdLine(_argc, _argv);

		m_width = _width;
		m_height = _height;
		m_debug = BGFX_DEBUG_NONE;//BGFX_DEBUG_STATS
		m_reset = BGFX_RESET_NONE;// BGFX_RESET_VSYNC;

		bgfx::init(args.m_type, args.m_pciId);
		bgfx::reset(m_width, m_height, m_reset);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		m_timeOffset = bx::getHPCounter();
		m_time = 0.0f;
		m_timeScale = 1.0f;

		Singleton<AgSceneDatabase>::instance().m_objectManager.destroy();

		const char* inputFileName = cmdLine.findOption('f');
#if BGFX_CONFIG_MULTITHREADED
		std::thread loadingThread(LoadFBX, inputFileName);
		loadingThread.detach();
#else
		if (!LoadFBX(filePath))
			assert(false);
#endif // BGFX_CONFIG_MULTITHREADED

		// TODO
		entry::setCurrentDir("runtime/");
		if (!Singleton<AgRenderResourceManager>::instance().init())
		{
			bgfx::dbgTextPrintf(0, 1, 0x0f, "Failed to initialize resource.");
		}

		AgCameraView* view0 = Singleton<AgRenderResourceManager>::instance().m_views.get(0);
		view0->setCamera(&m_camera0, AgRenderPass::E_VIEW_MAIN, m_width, m_height);
		view0->m_handle = 0;

		std::shared_ptr<AgLight> light(BX_NEW(entry::getAllocator(), AgSpotLight));
		light->m_position.m_x = 20.0f;
		light->m_position.m_y = 26.0f;
		light->m_position.m_z = 20.0f;
		Singleton<AgRenderResourceManager>::instance().m_lights.append(light);

		Singleton<AgRenderer>::instance().clearNodes();
		Singleton<AgRenderer>::instance().clearViews();
		//TODO:
		bgfx::setViewMode(AgRenderPass::E_VIEW_MAIN, bgfx::ViewMode::Sequential);

		m_timeOffset = bx::getHPCounter();
		m_bOcclusionQuery = false;
		m_bSelection = false;
		m_bSky = false;
		m_bShadow = false;
		m_secondViewer = false;
		m_occlusionThreshold = 1;

		imguiCreate();
	}

	int shutdown() override
	{
		imguiDestroy();

		Singleton<AgRenderer>::instance().destroy();
		Singleton<AgRenderResourceManager>::instance().destroy();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
			imguiBeginFrame(m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			showExampleDialog(this);

			ImGui::SetNextWindowPos(
				ImVec2(m_width - m_width / 5.0f - 10.0f, 10.0f)
				, ImGuiCond_FirstUseEver
			);
			ImGui::SetNextWindowSize(
				ImVec2(m_width / 5.0f, m_height / 2.0f)
				, ImGuiCond_FirstUseEver
			);
			ImGui::Begin("Settings"
				, NULL
				, 0
			);

			bgfx::TextureHandle debugTexture = Singleton<AgRenderer>::instance().getDebugTexture();
			if(bgfx::isValid(debugTexture))
				ImGui::Image(debugTexture, ImVec2(m_width / 5.0f - 16.0f, m_width / 5.0f - 16.0f));

			if (ImGui::Button("Focus"))
			{
				focusView(&m_camera0);
			}

			if (ImGui::Button("Clear"))
			{
				Singleton<AgRenderResourceManager>::instance().destroy();
			}

			const bgfx::Caps* caps = bgfx::getCaps();
			bool occlusionQuerySupported = 0 != (caps->supported & BGFX_CAPS_OCCLUSION_QUERY);
			if (occlusionQuerySupported)
			{
				ImGui::Checkbox("Occlusion Query", &m_bOcclusionQuery);
				ImGui::SliderInt("Occlusion Threshold", &m_occlusionThreshold, 1, 500);
				int32_t threshold = -2;
				if (occlusionQuerySupported && m_bOcclusionQuery)
					threshold = m_occlusionThreshold;
				Singleton<AgRenderer>::instance().enableOcclusionQuery(threshold);
			}
			
			bool blitSupport = 0 != (caps->supported & BGFX_CAPS_TEXTURE_BLIT);
			if (blitSupport)
			{
				ImGui::Checkbox("Hardware Selection", &m_bSelection);
				Singleton<AgRenderer>::instance().enableHardwarePicking(blitSupport && m_bSelection);
			}
			ImGui::Checkbox("Sky", &m_bSky);
			Singleton<AgRenderer>::instance().enableSkySystem(m_bSky);
			ImGui::SliderFloat("Time scale", &m_timeScale, 0.0f, 1.0f);
			ImGui::SliderFloat("Time", &m_time, 0.0f, 24.0f);

			ImGui::Checkbox("Shadow", &m_bShadow);
			Singleton<AgRenderer>::instance().enableShadow(m_bShadow);
			
			ImGui::Checkbox("Aux Viewer", &m_secondViewer);
			if (m_secondViewer)
			{
				AgCameraView* view1 = Singleton<AgRenderResourceManager>::instance().m_views.get(1);
				if (view1)
					view1->m_handle = 1;
			}
			else
			{
				AgCameraView* view1 = Singleton<AgRenderResourceManager>::instance().m_views.get(1);
				if (view1)
					view1->m_handle = AgCameraView::kInvalidHandle;
			}

			ImGui::End();

			imguiEndFrame();

			int64_t now = bx::getHPCounter();
			static int64_t last = now;
			const int64_t frameTime = now - last;
			last = now;
			const double freq = double(bx::getHPFrequency());
			const float deltaTime = float(frameTime / freq);
			m_time += m_timeScale * deltaTime;
			
			// Set view and projection matrix for view 0.
			const bgfx::HMD* hmd = bgfx::getHMD();
			if (NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING) )
			{
				//float view[16];
				//bx::mtxQuatTranslationHMD(view, hmd->eye[0].rotation, eye);
				//bgfx::setViewTransform(0, view, hmd->eye[0].projection, BGFX_VIEW_STEREO, hmd->eye[1].projection);

				//// Set view 0 default viewport.
				////
				//// Use HMD's width/height since HMD's internal frame buffer size
				//// might be much larger than window size.
				//bgfx::setViewRect(0, 0, 0, hmd->width, hmd->height);
			}
			else
			{
				if (!ImGui::MouseOverArea())
				{
					m_mouse.update(float(m_mouseState.m_mx), float(m_mouseState.m_my), m_mouseState.m_mz, m_width, m_height);
					if (m_mouseState.m_buttons[entry::MouseButton::Left])
					{
						m_camera0.orbit(m_mouse.m_dx, m_mouse.m_dy);
					}
					else if (m_mouseState.m_buttons[entry::MouseButton::Right])
					{
						m_camera0.dolly(m_mouse.m_dx + m_mouse.m_dy);
					}
					else if (0 != m_mouse.m_scroll)
					{
						m_camera0.dolly(float(m_mouse.m_scroll)*0.1f);
					}
				}
				m_camera0.update(deltaTime);

				{
					m_camera1 = m_camera0;
					m_camera1.reverseView();
				}

				if (m_mouseState.m_click)
				{
					m_mouseState.m_click = false;
					// Mouse coord in NDC
					float mouseXNDC = (m_mouseState.m_mx / (float)m_width) * 2.0f - 1.0f;
					float mouseYNDC = ((m_height - m_mouseState.m_my) / (float)m_height) * 2.0f - 1.0f;

					Singleton<AgRenderer>::instance().updatePickingInfo(mouseXNDC, mouseYNDC);
				}
			}

			AgCameraView* view0 = Singleton<AgRenderResourceManager>::instance().m_views.get(0);
			view0->setCamera(&m_camera0, AgRenderPass::E_VIEW_MAIN, m_width, m_height);
			AgCameraView* view1 = Singleton<AgRenderResourceManager>::instance().m_views.get(1);
			view1->setCamera(&m_camera1, AgRenderPass::E_VIEW_SECOND, m_width / 4, m_height / 4, 10, m_height - m_height / 4 - 10);
			//view1->m_handle = 1;

			Singleton<AgRenderer>::instance().updateTime(m_time);
			Singleton<AgRenderer>::instance().evaluateScene();
			Singleton<AgRenderer>::instance().runPipeline();

			return true;
		}

		return false;
	}

	struct Mouse
	{
		Mouse()
		{
			m_dx = 0.0f;
			m_dy = 0.0f;
			m_prevMx = 0.0f;
			m_prevMx = 0.0f;
			m_scroll = 0;
			m_scrollPrev = 0;
		}

		void update(float _mx, float _my, int32_t _mz, uint32_t _width, uint32_t _height)
		{
			const float widthf = float(int32_t(_width));
			const float heightf = float(int32_t(_height));

			// Delta movement.
			m_dx = float(_mx - m_prevMx) / widthf;
			m_dy = float(_my - m_prevMy) / heightf;

			m_prevMx = _mx;
			m_prevMy = _my;

			// Scroll.
			m_scroll = _mz - m_scrollPrev;
			m_scrollPrev = _mz;
		}

		float m_dx; // Screen space.
		float m_dy;
		float m_prevMx;
		float m_prevMy;
		int32_t m_scroll;
		int32_t m_scrollPrev;
	};
	Mouse				m_mouse;
	entry::MouseState	m_mouseState;

	AgCamera m_camera0;
	AgCamera m_camera1;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
	int64_t m_timeOffset;
	float m_time;
	float m_timeScale;

	bool m_bSelection;
	bool m_bSky;
	bool m_bShadow;
	bool m_bOcclusionQuery;
	int m_occlusionThreshold;
	bool m_secondViewer;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(BgFbxViewer, "04-mesh", "Loading meshes.");
