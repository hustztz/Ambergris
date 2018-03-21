#include "BGFX/common.h"
#include "BGFX/bgfx_utils.h"
#include "BGFX/imgui/bgfx_imgui.h"

#include <bx/commandline.h>

#include "Render/AgRenderer.h"
#include "Resource/AgRenderResourceManager.h"
#include "FBX/FbxImportManager.h"
#include "Scene/AgSceneDatabase.h"
#include "Scene/AgCamera.h"

#include <algorithm>
#include <thread>

using namespace ambergris;

#define FAR_VIEW_CLIP 10000.0f
#define NEAR_VIEW_CLIP 0.1f

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

	void focusView()
	{
		const AgSceneDatabase& sceneDB = Singleton<AgSceneDatabase>::instance();

		bool touched = false;
		Aabb sceneAabb;
		sceneAabb.m_min[0] = std::numeric_limits<float>::max();
		sceneAabb.m_min[1] = std::numeric_limits<float>::max();
		sceneAabb.m_min[2] = std::numeric_limits<float>::max();
		sceneAabb.m_max[0] = std::numeric_limits<float>::min();
		sceneAabb.m_max[1] = std::numeric_limits<float>::min();
		sceneAabb.m_max[2] = std::numeric_limits<float>::min();
		const int nNodeNum = (const int)sceneDB.getSize();
		for (int i = 0; i < nNodeNum; i++)
		{
			const AgMesh* node = dynamic_cast<const AgMesh*>(sceneDB.get(i));
			if (!node || !node->m_bb.m_initialized)
				continue;
			Aabb nodeAabb = node->m_bb.m_aabb;
			float tmpAabb[4];
			tmpAabb[0] = nodeAabb.m_min[0];
			tmpAabb[1] = nodeAabb.m_min[1];
			tmpAabb[2] = nodeAabb.m_min[2];
			tmpAabb[3] = 1.0f;
			float nodeMin[4];
			bx::vec4MulMtx(nodeMin, tmpAabb, node->m_global_transform);
			tmpAabb[0] = nodeAabb.m_max[0];
			tmpAabb[1] = nodeAabb.m_max[1];
			tmpAabb[2] = nodeAabb.m_max[2];
			float nodeMax[4];
			bx::vec4MulMtx(nodeMax, tmpAabb, node->m_global_transform);

			if (nodeMin[0] < sceneAabb.m_min[0])
				sceneAabb.m_min[0] = nodeMin[0];
			if (nodeMin[1] < sceneAabb.m_min[1])
				sceneAabb.m_min[1] = nodeMin[1];
			if (nodeMin[2] < sceneAabb.m_min[2])
				sceneAabb.m_min[2] = nodeMin[2];
			if (nodeMax[0] > sceneAabb.m_max[0])
				sceneAabb.m_max[0] = nodeMax[0];
			if (nodeMax[1] > sceneAabb.m_max[1])
				sceneAabb.m_max[1] = nodeMax[1];
			if (nodeMax[2] > sceneAabb.m_max[2])
				sceneAabb.m_max[2] = nodeMax[2];
			touched = true;
		}
		if (!touched)
			return;

		float scale = std::max((float)(sceneAabb.m_max[0] - sceneAabb.m_min[0]), std::max((float)(sceneAabb.m_max[1] - sceneAabb.m_min[1]), (float)(sceneAabb.m_max[2] - sceneAabb.m_min[2])));
		float target[3] = { (sceneAabb.m_max[0] + sceneAabb.m_min[0]) * 0.5f, (sceneAabb.m_max[1] + sceneAabb.m_min[1]) * 0.5f, (sceneAabb.m_max[2] + sceneAabb.m_min[2]) * 0.5f };
		float eye[3] = { scale, 0.0f, 0.0f };

		m_camera.setTarget(target);
		m_camera.setPosition(eye);
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

		Singleton<AgSceneDatabase>::instance().destroy();

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
		Singleton<AgRenderer>::instance().clearNodes();
		Singleton<AgRenderer>::instance().m_viewPass.m_width = m_width;
		Singleton<AgRenderer>::instance().m_viewPass.m_height = m_height;
		Singleton<AgRenderer>::instance().m_viewPass.init(AgRenderPass::E_VIEW_MAIN);
		//TODO:
		bgfx::setViewMode(AgRenderPass::E_VIEW_MAIN, bgfx::ViewMode::Sequential);

		m_timeOffset = bx::getHPCounter();
		m_bOcclusionQuery = false;
		m_bSelection = false;
		m_bSky = false;
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
				focusView();
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
			
			ImGui::Checkbox("Aux Viewer", &m_secondViewer);
			if (m_secondViewer)
			{
				Singleton<AgRenderer>::instance().m_viewPass.init(AgRenderPass::E_VIEW_SECOND);
			}
			else
			{
				Singleton<AgRenderer>::instance().m_viewPass.m_pass_state[AgRenderPass::E_VIEW_SECOND].isValid = false;
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
						m_camera.orbit(m_mouse.m_dx, m_mouse.m_dy);
					}
					else if (m_mouseState.m_buttons[entry::MouseButton::Right])
					{
						m_camera.dolly(m_mouse.m_dx + m_mouse.m_dy);
					}
					else if (0 != m_mouse.m_scroll)
					{
						m_camera.dolly(float(m_mouse.m_scroll)*0.1f);
					}
				}

				m_camera.update(deltaTime);
				float view[16];
				m_camera.getViewMtx(view);

				const float camFovy = 60.0f;
				const float camAspect = float(m_width) / float(m_height);
				float proj[16];
				bx::mtxProj(proj, camFovy, camAspect, NEAR_VIEW_CLIP, FAR_VIEW_CLIP, true);// bgfx::getCaps()->homogeneousDepth);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_MAIN, view, proj);

				float backView[16];
				m_camera.getBackViewMtx(backView);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_SECOND, backView, proj);

				if (m_mouseState.m_click)
				{
					m_mouseState.m_click = false;
					// Set up picking pass
					float viewProj[16];
					bx::mtxMul(viewProj, view, proj);

					float invViewProj[16];
					bx::mtxInverse(invViewProj, viewProj);

					// Mouse coord in NDC
					float mouseXNDC = (m_mouseState.m_mx / (float)m_width) * 2.0f - 1.0f;
					float mouseYNDC = ((m_height - m_mouseState.m_my) / (float)m_height) * 2.0f - 1.0f;

					Singleton<AgRenderer>::instance().updatePickingView(invViewProj, mouseXNDC, mouseYNDC, 3.0f, NEAR_VIEW_CLIP, FAR_VIEW_CLIP);
				}
				const float projHeight = 1.0f / bx::tan(bx::toRad(camFovy)*0.5f);
				const float projWidth = projHeight * camAspect;
				Singleton<AgRenderer>::instance().updateLights(view, projWidth, projHeight);
			}

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
	AgCamera			m_camera;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
	int64_t m_timeOffset;
	float m_time;
	float m_timeScale;

	bool m_bSelection;
	bool m_bSky;
	bool m_bOcclusionQuery;
	int m_occlusionThreshold;
	bool m_secondViewer;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(BgFbxViewer, "04-mesh", "Loading meshes.");
