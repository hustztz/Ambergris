#pragma once

#include "BGFX/font/font_manager.h"
#include "BGFX/font/text_buffer_manager.h"

#include <iconfontheaders/icons_font_awesome.h>
#include <iconfontheaders/icons_kenney.h>

#include <mutex>

namespace ambergris {

	class AgTextManager
	{
	public:
		AgTextManager();
		~AgTextManager();

		bool init();
		void destroy();
		void appendDynamic(const char* _string, bool replace = true);
		void appendDynamic(const wchar_t* _string, bool replace = true);
		void submit(bgfx::ViewId view_id);
	protected:
		void _InitText();
		bool _InitFont();
	private:
		FontManager* m_fontManager;
		TextBufferManager* m_textBufferManager;

		FontHandle m_dynamicFont;
		TextBufferHandle m_dynamicText;
		mutable std::mutex		m_mutex;
	};
}