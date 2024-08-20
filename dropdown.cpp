#include "includes.h"

void Dropdown::arrow1(Point p, bool is_hovered) {
	Color arrow_color = is_hovered ? g_gui.m_color : Color{ 152, 152, 152, m_parent->m_alpha };

	render::rect_filled(p.x + m_w - 11, p.y - 2 + m_offset + 9, 5, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
	render::rect_filled(p.x + m_w - 11, p.y - 1 + m_offset + 9, 5, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
	render::rect_filled(p.x + m_w - 10, p.y - 1 + m_offset + 9 + 1, 3, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
	render::rect_filled(p.x + m_w - 9, p.y - 1 + m_offset + 9 + 2, 1, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
}

void Dropdown::arrow2( Point l ) {
	render::rect_filled( l.x + m_w - 11, l.y - 2 + m_offset + 9, 5, 1, { 0,0,0, m_parent->m_alpha });
	render::rect_filled( l.x + m_w - 11, l.y - 1 + m_offset + 9, 5, 1, { 152, 152, 152, m_parent->m_alpha } );
	render::rect_filled( l.x + m_w - 10, l.y - 1 + m_offset + 9 + 1, 3, 1, { 152, 152, 152, m_parent->m_alpha } );
	render::rect_filled( l.x + m_w - 9, l.y - 1 + m_offset + 9 + 2, 1, 1, { 152, 152, 152, m_parent->m_alpha } );
}

void Dropdown::draw() {
	Rect area{ m_parent->GetElementsRect() };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };
	Rect bar{ p.x + DROPDOWN_X_OFFSET, p.y + m_offset, m_w - DROPDOWN_X_OFFSET, DROPDOWN_BOX_HEIGHT };
	bool is_hovered = g_input.IsCursorInRect(bar);

	// Define the maximum width for the selected items text in the bar.
	int maxTextWidth = m_w - DROPDOWN_X_OFFSET - DROPDOWN_ITEM_X_OFFSET - 10;

	Color color = g_gui.m_color;
	color.a() = m_parent->m_alpha;

	if (m_use_label)
		render::menu_shade.string(p.x + LABEL_OFFSET, p.y - 2, { 205, 205, 205, m_parent->m_alpha }, m_label);

	render::rect(p.x + DROPDOWN_X_OFFSET, p.y + m_offset, m_w - DROPDOWN_X_OFFSET, DROPDOWN_BOX_HEIGHT, { 0, 0, 0, m_parent->m_alpha });

	render::rect_filled(p.x + DROPDOWN_X_OFFSET + 1, p.y + m_offset + 1, m_w - DROPDOWN_X_OFFSET - 2, DROPDOWN_BOX_HEIGHT - 2, { 41, 41, 41, m_parent->m_alpha });

	if (g_input.IsCursorInRect(bar) || m_open) {
		render::gradient2(p.x + DROPDOWN_X_OFFSET + 1, p.y + m_offset + 1, m_w - DROPDOWN_X_OFFSET - 2, DROPDOWN_BOX_HEIGHT - 2, { 46, 46, 46, m_parent->m_alpha }, { 51, 51, 51, m_parent->m_alpha });
	}
	else {
		render::gradient2(p.x + DROPDOWN_X_OFFSET + 1, p.y + m_offset + 1, m_w - DROPDOWN_X_OFFSET - 2, DROPDOWN_BOX_HEIGHT - 2, { 36, 36, 36, m_parent->m_alpha }, { 41, 41, 41, m_parent->m_alpha });
	}

	arrow1(p, is_hovered);

	if (!m_items.empty()) {
		if (m_open) {
			render::rect(p.x + DROPDOWN_X_OFFSET, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR, m_w - DROPDOWN_X_OFFSET, m_anim_height + 1, { 0, 0, 0, m_parent->m_alpha });
			render::rect_filled(p.x + DROPDOWN_X_OFFSET + 1, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR + 1, m_w - DROPDOWN_X_OFFSET - 2, m_anim_height - 1, { 41, 41, 41, m_parent->m_alpha });

			// Iterate through all items and display them.
			for (size_t i = 0; i < m_items.size(); ++i) {
				int item_offset = (i * DROPDOWN_ITEM_HEIGHT);
				if (m_anim_height > item_offset) {
					bool active = (i == m_active_item);
					bool hovered = g_input.IsCursorInRect({ p.x + DROPDOWN_X_OFFSET, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR + item_offset, m_w - DROPDOWN_X_OFFSET, DROPDOWN_ITEM_HEIGHT });

					Color itemColor = active ? g_gui.m_color : (hovered ? Color{ 235, 235, 235, m_parent->m_alpha } : Color{ 152, 152, 152, m_parent->m_alpha });

					render::menu.string(p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_ITEM_Y_OFFSET + item_offset, itemColor, m_items[i]);
				}
			}
		}
		else {
			// Check if there are no items, and display "none" if that's the case.
			if (m_active_item == -1)
				render::menu.string(p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET, p.y + m_offset + 4, { 152, 152, 152, m_parent->m_alpha }, XOR("none"));
			else if (m_active_item >= 0 && m_active_item < static_cast<int>(m_items.size()))
				render::menu.string(p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET, p.y + m_offset + 4, { 152, 152, 152, m_parent->m_alpha }, m_items[m_active_item]);
		}
	}
}


void Dropdown::think( ) {
	// fucker can be opened.
	if( !m_items.empty( ) ) {
		int total_size = DROPDOWN_ITEM_HEIGHT * m_items.size( );

		// we need to travel 'total_size' in 300 ms.
		float frequency = total_size / 0.01f;

		// the increment / decrement per frame.
		float step = frequency * g_csgo.m_globals->m_frametime;

		// if open		-> increment
		// if closed	-> decrement
		m_open ? m_anim_height += step : m_anim_height -= step;

		// clamp the size.
		math::clamp< float >( m_anim_height, 0, total_size );

		// open
		if( m_open ) {
			// clicky height.
			m_h = m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR + total_size;

			// another element is in focus.
			if( m_parent->m_active_element != this )
				m_open = false;
		}

		// closed.
		else
			m_h = m_offset + DROPDOWN_BOX_HEIGHT;
	}

	// no items, no open.
	else
		m_h = m_offset + DROPDOWN_BOX_HEIGHT;
}

void Dropdown::click( ) {
	Rect  area{ m_parent->GetElementsRect( ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// bar.
	Rect bar{ p.x + DROPDOWN_X_OFFSET, p.y + m_offset, m_w - DROPDOWN_X_OFFSET, DROPDOWN_BOX_HEIGHT };

	// pop open if bar pressed
	if( g_input.IsCursorInRect( bar ) )
		m_open = true;

	// check item clicks.
	if( m_open ) {
		if( !g_input.IsCursorInRect( bar ) ) {
			// iterate items.
			for( size_t i{}; i < m_items.size( ); ++i ) {
				Rect item{ p.x + DROPDOWN_X_OFFSET, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR + ( i * DROPDOWN_ITEM_HEIGHT ), m_w - DROPDOWN_X_OFFSET, DROPDOWN_ITEM_HEIGHT };

				// click was in context of current item.
				if( g_input.IsCursorInRect( item ) ) {
					m_active_item = i;

					if( m_callback )
						m_callback( );
				}
			}
		}
	}
}