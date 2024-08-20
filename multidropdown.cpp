#include "includes.h"

void MultiDropdown::arrow1(Point p, bool is_hovered) {
    Color arrow_color = is_hovered ? g_gui.m_color : Color{ 152, 152, 152, m_parent->m_alpha }; 

    render::rect_filled(p.x + m_w - 11, p.y - 2 + m_offset + 9, 5, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
    render::rect_filled(p.x + m_w - 11, p.y - 1 + m_offset + 9, 5, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
    render::rect_filled(p.x + m_w - 10, p.y - 1 + m_offset + 9 + 1, 3, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
    render::rect_filled(p.x + m_w - 9, p.y - 1 + m_offset + 9 + 2, 1, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
}


void MultiDropdown::arrow2(Point p, bool is_hovered) {
    Color arrow_color = is_hovered ? g_gui.m_color : Color{ 152, 152, 152, m_parent->m_alpha };
    arrow_color.a() = m_parent->m_alpha; 

    render::rect_filled(p.x + m_w - 11, p.y + 2 + m_offset + 9, 5, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
    render::rect_filled(p.x + m_w - 11, p.y + 1 + m_offset + 9, 5, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
    render::rect_filled(p.x + m_w - 10, p.y + 1 + m_offset + 9 - 1, 3, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
    render::rect_filled(p.x + m_w - 9, p.y + 1 + m_offset + 9 - 2, 1, 1, { arrow_color.r(), arrow_color.g(), arrow_color.b(), arrow_color.a() });
}

void MultiDropdown::renderSelectedItemsInBar(Point p, Color color, int maxTextWidth) {
    std::stringstream selected_items_text;

    for (const auto& item : m_active_items) {
        size_t index = item.first;
        if (index < m_items.size()) {
            if (!selected_items_text.str().empty()) {
                selected_items_text << ", ";
            }
            selected_items_text << m_items[index];
        }
    }

    std::string text = selected_items_text.str();
    int maxTextWidthWithoutEllipsis = maxTextWidth - render::menu.size(XOR("...")).m_width;

    if (render::menu.size(text).m_width > maxTextWidth) {
        while (render::menu.size(text + XOR("...")).m_width > maxTextWidth) {
            text.pop_back(); 
        }
        text += XOR("..."); 
    }
    
    render::menu.string(p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET, p.y + m_offset + 4, { 152, 152, 152, m_parent->m_alpha }, text);
}

void MultiDropdown::draw() {
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
                    auto it = std::find_if(m_active_items.begin(), m_active_items.end(),
                        [i](const std::pair<size_t, size_t>& element) { return element.first == i; });

                    bool active = it != m_active_items.end();
                    bool hovered = g_input.IsCursorInRect({ p.x + DROPDOWN_X_OFFSET, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR + item_offset, m_w - DROPDOWN_X_OFFSET, DROPDOWN_ITEM_HEIGHT });

                    Color itemColor = active ? g_gui.m_color : (hovered ? Color{ 235, 235, 235, m_parent->m_alpha } : Color{ 152, 152, 152, m_parent->m_alpha });

                    std::string display_text = active ? "{" + std::to_string(it->second) + "} ~ " + m_items[i] : m_items[i];

                    render::menu.string(p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_ITEM_Y_OFFSET + item_offset, itemColor, display_text);
                }
            }
        }
        else {
            // Check if there are no items, and display "none" if that's the case.
            if (m_active_items.empty())
                render::menu.string(p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET, p.y + m_offset + 4, { 152, 152, 152, m_parent->m_alpha }, XOR("none"));
            else if (m_active_items.size() == 1)
                render::menu.string(p.x + DROPDOWN_X_OFFSET + DROPDOWN_ITEM_X_OFFSET, p.y + m_offset + 4, { 152, 152, 152, m_parent->m_alpha }, m_items[m_active_items[0].first]);
        }

        // Call the function to render the selected items in the bar above the dropdown.
        renderSelectedItemsInBar(p, color, maxTextWidth);
    }
}


void MultiDropdown::think( ) {
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

void MultiDropdown::click() {
    Rect area{ m_parent->GetElementsRect() };
    Point p{ area.x + m_pos.x, area.y + m_pos.y };
    Rect bar{ p.x + DROPDOWN_X_OFFSET, p.y + m_offset, m_w - DROPDOWN_X_OFFSET, DROPDOWN_BOX_HEIGHT };

    if (g_input.IsCursorInRect(bar))
        m_open = !m_open;

    if (m_open) {
        for (size_t i = 0; i < m_items.size(); ++i) {
            Rect item{ p.x + DROPDOWN_X_OFFSET, p.y + m_offset + DROPDOWN_BOX_HEIGHT + DROPDOWN_SEPARATOR + (i * DROPDOWN_ITEM_HEIGHT), m_w - DROPDOWN_X_OFFSET, DROPDOWN_ITEM_HEIGHT };

            if (g_input.IsCursorInRect(item)) {
                auto it = std::find_if(m_active_items.begin(), m_active_items.end(),
                    [i](const std::pair<size_t, size_t>& element) { return element.first == i; });

                if (it != m_active_items.end()) {
                    // Item is currently selected; remove it.
                    size_t removedOrder = it->second;
                    m_active_items.erase(it);

                    // Adjust the order of all items with a higher order.
                    for (auto& item : m_active_items) {
                        if (item.second > removedOrder) {
                            --item.second;
                        }
                    }
                }
                else {
                    // Item is not selected; add it with the correct order.
                    size_t order = m_active_items.empty() ? 1 : m_active_items.back().second + 1;
                    m_active_items.emplace_back(i, order);
                }

                if (m_callback)
                    m_callback();
            }
        }
    }
}