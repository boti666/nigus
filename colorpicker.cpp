#include "includes.h"

// static variables.
int Colorpicker::texture = 0;;
std::unique_ptr< Color[] > Colorpicker::gradient = nullptr;;

void Colorpicker::init() {
	const int w{ COLORPICKER_PICKER_SIZE };
	const int h{ COLORPICKER_PICKER_SIZE };

	// should never happen.
	if (gradient)
		return;

	// allocate.
	gradient = std::make_unique< Color[] >(w * h);

	// init.
	float hue{}, sat{ 0.99f }, lum{ 1.f };

	// iterate width.
	for (int i{}; i < w; ++i) {

		// iterate height.
		for (int j{}; j < h; ++j) {

			// write back to array.
			*(Color*)(gradient.get() + i + j * w) = Color::hsl_to_rgb(hue, sat, lum);

			hue += (1.f / w);
		}

		lum -= (1.f / h);
		hue = 0.f;
	}

	// allocate new texture in engine.
	texture = g_csgo.m_surface->CreateNewTextureID(true);

	// assign allocated memory containing the picker to said texture.
	g_csgo.m_surface->DrawSetTextureRGBA(texture, gradient.get(), w, h);
}

void Colorpicker::draw() {
	Rect  area{ m_parent->GetElementsRect() };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };
	Rect preview_rect{ p.x + m_w - COLORPICKER_WIDTH, p.y, COLORPICKER_WIDTH, COlORPICKER_HEIGHT };

	// label.
	Color col = g_gui.m_copied_file_id == m_file_id ? g_gui.m_color : Color(205, 205, 205, m_parent->m_alpha);
	render::menu_shade.string(p.x + LABEL_OFFSET, p.y - 2, col, m_label);

	// outline.
	Color brder = g_gui.m_copied_file_id == m_file_id ? g_gui.m_color : Color(0, 0, 0, m_parent->m_alpha);
	render::rect(p.x + m_w - COLORPICKER_WIDTH, p.y, COLORPICKER_WIDTH, COlORPICKER_HEIGHT, brder);

	// make a copy of the color.
	Color preview = m_color;

	// draw preview window.
	render::rect_filled(p.x + m_w - COLORPICKER_WIDTH + 1, p.y + 1, COLORPICKER_WIDTH - 2, COlORPICKER_HEIGHT - 2, preview);

	// fade / shine.
	render::rect_filled_fade(p.x + m_w - COLORPICKER_WIDTH + 1, p.y + 1, COLORPICKER_WIDTH - 2, COlORPICKER_HEIGHT - 2, { 50, 50, 35, m_parent->m_alpha }, 0, 150);

	if (g_input.IsCursorInRect(preview_rect)) {
		// alpha label.
		render::menu_shade.string(preview_rect.x + LABEL_OFFSET, p.y - 2, { 205, 205, 205, m_parent->m_alpha }, tfm::format("%i", m_color.a()));
	}

	if (g_gui.m_copying) {
		m_parent->m_alpha = 255.f;
	}

	// draw picker
	if (m_open) {
		// set panel color.
		g_csgo.m_surface->DrawSetColor(colors::white);

		// set texture.
		g_csgo.m_surface->DrawSetTexture(texture);

		// draw picker.
		g_csgo.m_surface->DrawTexturedRect(p.x + m_w - COLORPICKER_WIDTH,
			p.y + COlORPICKER_HEIGHT + 2,
			p.x + m_w - COLORPICKER_WIDTH + COLORPICKER_PICKER_SIZE,
			p.y + COlORPICKER_HEIGHT + 2 + COLORPICKER_PICKER_SIZE);

		// picker outline.
		render::rect(p.x + m_w - COLORPICKER_WIDTH,
			p.y + COlORPICKER_HEIGHT + 2,
			COLORPICKER_PICKER_SIZE,
			COLORPICKER_PICKER_SIZE,
			{ 0, 0, 0, m_parent->m_alpha });

	}
}

void Colorpicker::think() {
	Rect  area{ m_parent->GetElementsRect() };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };
	Rect preview{ p.x + m_w - COLORPICKER_WIDTH, p.y, COLORPICKER_WIDTH, COlORPICKER_HEIGHT };

	if (g_input.IsCursorInRect(preview)) {
		if (g_input.m_scroll == 120) { // up
			m_color.a() += 1;
		}
		else if (g_input.m_scroll == -120) { // down
			m_color.a() -= 1;
		}

		if (g_input.GetKeyState(VK_DOWN) || g_input.GetKeyState(VK_LEFT)) {
			m_color.a() -= 1;
		}
		else if (g_input.GetKeyState(VK_UP) || g_input.GetKeyState(VK_RIGHT)) {
			m_color.a() += 1;
		}

		g_input.m_scroll = 0;

		if (g_input.GetKeyState(VK_CONTROL) && g_input.GetKeyState(VK_LBUTTON) && !g_gui.m_copying && g_csgo.m_globals->m_frame - g_gui.m_last_copied_frame > 50) {
			g_gui.m_copying = true;
			g_gui.m_copied_color = m_color;
			g_gui.m_last_copied_frame = g_csgo.m_globals->m_frame;
			g_gui.m_copied_file_id = m_file_id;
		}

		if (g_input.GetKeyState(VK_CONTROL) && g_input.GetKeyPress(VK_LBUTTON) && g_gui.m_copying && g_csgo.m_globals->m_frame - g_gui.m_last_copied_frame > 50) {
			m_color = g_gui.m_copied_color;
			g_gui.m_copying = false;
			g_gui.m_last_copied_frame = g_csgo.m_globals->m_frame;
			g_gui.m_copied_file_id = XOR("StinkyFartyPenis97");
		}
	}

	std::clamp(m_color.a(), (uint8_t)0, (uint8_t)255);

	if (m_open) {
		g_gui.m_should_drag = false; // doesnt work but it isnt that much of an issue

		Rect picker{ p.x + m_w - COLORPICKER_WIDTH, p.y + COlORPICKER_HEIGHT + 2, COLORPICKER_PICKER_SIZE, COLORPICKER_PICKER_SIZE };

		if (g_input.IsCursorInRect(picker) && g_input.GetKeyState(VK_LBUTTON)) {
			m_parent->m_active_element = this;
			float old_a = m_color.a();
			m_color = Colorpicker::ColorFromPos(g_input.m_mouse.x - (p.x + m_w - COLORPICKER_WIDTH), g_input.m_mouse.y - (p.y + COlORPICKER_HEIGHT + 2));
			m_color.a() = old_a;
		}

		if (m_parent->m_active_element != this) {

			if (!g_input.IsCursorInRect(picker))
				m_open = false;

			if (m_callback)
				m_callback();
		}
	}

	if (m_ptr)
		*m_ptr = m_color;
}

void Colorpicker::click() {
	Rect  area{ m_parent->GetElementsRect() };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	Rect preview{ p.x + m_w - COLORPICKER_WIDTH, p.y, COLORPICKER_WIDTH, COlORPICKER_HEIGHT };

	if (g_input.IsCursorInRect(preview) && !g_input.GetKeyState(VK_CONTROL))
		m_open = true;
}