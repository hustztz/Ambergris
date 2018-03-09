#include "AgTextManager.h"

#include "Foundation/AgFileUtils.h"

#include "BGFX/entry/entry.h"//TODO

namespace ambergris {

	TrueTypeHandle loadTtf(FontManager* _fm, const char* _filePath)
	{
		uint32_t size;
		void* data = fileUtils::load(_filePath, &size);

		if (NULL != data)
		{
			TrueTypeHandle handle = _fm->createTtf((uint8_t*)data, size);
			BX_FREE(entry::getAllocator(), data);
			return handle;
		}

		TrueTypeHandle invalid = BGFX_INVALID_HANDLE;
		return invalid;
	}

	static const char* s_fontFilePath[] =
	{
		"font/droidsans.ttf",
		"font/chp-fire.ttf",
		"font/bleeding_cowboys.ttf",
		"font/mias_scribblings.ttf",
		"font/ruritania.ttf",
		"font/signika-regular.ttf",
		"font/five_minutes.otf",
	};

	AgTextManager::AgTextManager()
		: m_textBufferManager(nullptr)
		, m_fontManager(nullptr)
		, m_dynamicText(BGFX_INVALID_HANDLE)
		, m_dynamicFont(BGFX_INVALID_HANDLE)
	{
		
	}

	AgTextManager::~AgTextManager()
	{
		destroy();
	}

	bool AgTextManager::init()
	{
		// Init the text rendering system.
		m_fontManager = new FontManager(512);
		m_textBufferManager = new TextBufferManager(m_fontManager);
		if (!_InitFont())
			return false;
		_InitText();
		return m_textBufferManager && bgfx::kInvalidHandle != m_dynamicText.idx && bgfx::kInvalidHandle != m_dynamicFont.idx;
	}

	void AgTextManager::destroy()
	{
		if (m_textBufferManager)
		{
			m_fontManager->destroyFont(m_dynamicFont);
			m_textBufferManager->destroyTextBuffer(m_dynamicText);
			delete m_textBufferManager;
			m_textBufferManager = nullptr;
		}
		if (m_fontManager)
		{
			delete m_fontManager;
			m_fontManager = nullptr;
		}
	}

	bool AgTextManager::_InitFont()
	{
		if (!m_fontManager)
			return false;
		// TODO
		entry::setCurrentDir("runtime/");
		// Instantiate a usable font.
		TrueTypeHandle dynamicFontFile = loadTtf(m_fontManager, s_fontFilePath[0]);
		if (bgfx::kInvalidHandle == dynamicFontFile.idx)
			return false;
		m_dynamicFont = m_fontManager->createFontByPixelSize(dynamicFontFile, 0, 32);
		if (bgfx::kInvalidHandle == m_dynamicFont.idx)
			return false;
		// Preload glyphs and blit them to atlas.
		m_fontManager->preloadGlyph(m_dynamicFont, L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ. \n");

		// You can unload the truetype files at this stage, but in that
		// case, the set of glyph's will be limited to the set of preloaded
		// glyph.
		if(bgfx::kInvalidHandle == dynamicFontFile.idx)
			m_fontManager->destroyTtf(dynamicFontFile);

		return true;
	}

	void AgTextManager::_InitText()
	{
		if (!m_textBufferManager)
			return;

		m_dynamicText = m_textBufferManager->createTextBuffer(FONT_TYPE_ALPHA, BufferType::Dynamic);

		// Setup style colors.
		//m_textBufferManager->setBackgroundColor(m_dynamicText, 0x551111ff);
		//m_textBufferManager->setUnderlineColor(m_dynamicText, 0xff2222ff);
		//m_textBufferManager->setOverlineColor(m_dynamicText, 0x2222ffff);
		//m_textBufferManager->setStrikeThroughColor(m_dynamicText, 0x22ff22ff);

		//// Background.
		//m_textBufferManager->setStyle(m_dynamicText, STYLE_BACKGROUND);

		// The pen position represent the top left of the box of the first line
		// of text.
		m_textBufferManager->setPenPosition(m_dynamicText, 24.0f, 100.0f);
	}

	void AgTextManager::appendDynamic(const char* _string, bool replace /*= true*/)
	{
		if (!m_textBufferManager || bgfx::kInvalidHandle == m_dynamicText.idx || bgfx::kInvalidHandle == m_dynamicFont.idx)
			return;

		std::lock_guard<std::mutex> lck(m_mutex);
		if (replace)
		{
			m_textBufferManager->clearTextBuffer(m_dynamicText);
		}
		m_textBufferManager->appendText(m_dynamicText, m_dynamicFont, _string);
	}

	void AgTextManager::appendDynamic(const wchar_t* _string, bool replace /*= true*/)
	{
		if (!m_textBufferManager || bgfx::kInvalidHandle == m_dynamicText.idx || bgfx::kInvalidHandle == m_dynamicFont.idx)
			return;

		std::lock_guard<std::mutex> lck(m_mutex);
		if (replace)
		{
			m_textBufferManager->clearTextBuffer(m_dynamicText);
		}
		m_textBufferManager->appendText(m_dynamicText, m_dynamicFont, _string);
	}

	void AgTextManager::submit(bgfx::ViewId view_id)
	{
		if (!m_textBufferManager || bgfx::kInvalidHandle == m_dynamicText.idx)
			return;
		std::lock_guard<std::mutex> lck(m_mutex);
		// Submit the static text.
		//m_textBufferManager->submitTextBuffer(m_dynamicText, view_id);
	}
}