#include "BGFX/common.h"
#include "BGFX/bgfx_utils.h"
#include "BGFX/camera.h"
#include "BGFX/imgui/bgfx_imgui.h"

#include "Render/AgRenderer.h"
#include "Render/AgRenderMeshEvaluator.h"
#include "FBX/FbxImportManager.h"
#include "Scene/AgSceneDatabase.h"

#include <algorithm>

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

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_NONE;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::init(args.m_type, args.m_pciId);
		bgfx::reset(m_width, m_height, m_reset);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		u_time = bgfx::createUniform("u_time", bgfx::UniformType::Vec4);

		ambergris_fbx::FbxImportManager fbxManager;

		char filePath[] = "C:\\Users\\hustztz\\Documents\\3dsMax\\export\\test2.FBX";
		if (!fbxManager.Load(filePath))
		{
			//std::cout << "An error occurred while loading the scene..." << std::endl;
			return;
		}

		const AgSceneDatabase& sceneDB = Singleton<AgSceneDatabase>::instance();

		ambergris_fbx::FbxImportManager::SmtFbxMeshCBFunc meshCB(std::bind(&AddMeshToRenderer, std::placeholders::_1));
		fbxManager.RegisterMeshCBFunc(meshCB);

		if (!fbxManager.ParseScene())
			return;

		AgRenderer& renderer = Singleton<AgRenderer>::instance();
		renderer.Init(entry::getFileReader() );
		if (!AgRenderSceneBridge(renderer, sceneDB))
			return;

		m_timeOffset = bx::getHPCounter();

		imguiCreate();
		cameraCreate();

		Aabb sceneAabb;
		sceneAabb.m_min[0] = std::numeric_limits<float>::max();
		sceneAabb.m_min[1] = std::numeric_limits<float>::max();
		sceneAabb.m_min[2] = std::numeric_limits<float>::max();
		sceneAabb.m_max[0] = std::numeric_limits<float>::min();
		sceneAabb.m_max[1] = std::numeric_limits<float>::min();
		sceneAabb.m_max[2] = std::numeric_limits<float>::min();
		const int nNodeNum = (const int)sceneDB.GetSize();
		for (int i = 0; i < nNodeNum; i++)
		{
			const AgMesh* node = dynamic_cast<const AgMesh*>(sceneDB.Get(i));
			if(!node)
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
		}
		float scale = std::max((float)(sceneAabb.m_max[0] - sceneAabb.m_min[0]), std::max((float)(sceneAabb.m_max[1] - sceneAabb.m_min[1]), (float)(sceneAabb.m_max[2] - sceneAabb.m_min[2])));
		float at[3] = { (sceneAabb.m_max[0] + sceneAabb.m_min[0]) * 0.5f, (sceneAabb.m_max[1] + sceneAabb.m_min[1]) * 0.5f, (sceneAabb.m_max[2] + sceneAabb.m_min[2]) * 0.5f };
		float eye[3] = { scale, 0.0f, 0.0f };

		cameraSetFocus(at);
		cameraSetPosition(eye);
		//cameraSetVerticalAngle(-bx::kPi / 4.0f);
	}

	int shutdown() override
	{
		cameraDestroy();
		imguiDestroy();

		Singleton<AgRenderer>::instance().Destroy();

		bgfx::destroy(u_time);

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

			static float distance = 1.0f;
			ImGui::SliderFloat("Distance", &distance, 1.0f, 10.0f);

			ImGui::End();

			imguiEndFrame();

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			float time = (float)( (bx::getHPCounter()-m_timeOffset)/double(bx::getHPFrequency() ) );
			bgfx::setUniform(u_time, &time);

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
				float view[16];
				cameraGetViewMtx(view);

				float proj[16];
				bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 10000.0f, true);// bgfx::getCaps()->homogeneousDepth);
				bgfx::setViewTransform(0, view, proj);

				// Set view 0 default viewport.
				bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
			}

			int64_t now = bx::getHPCounter();
			static int64_t last = now;
			const int64_t frameTime = now - last;
			last = now;
			const double freq = double(bx::getHPFrequency());
			const float deltaTime = float(frameTime / freq);
			if (!ImGui::MouseOverArea())
			{
				// Update camera.
				cameraUpdate(deltaTime, m_mouseState);
			}

			Singleton<AgRenderer>::instance().Draw();

			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	entry::MouseState m_mouseState;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;

	int64_t m_timeOffset;
	bgfx::UniformHandle u_time;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(BgFbxViewer, "04-mesh", "Loading meshes.");
