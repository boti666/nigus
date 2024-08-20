#include "includes.h"
#include "font.h"

namespace render {
	Font menu;;
	Font menu_shade;;
	Font esp;;
	Font esp2;;
	Font esp_small;;
	Font esp_small2;;
	Font hud;;
	Font cs;;
	Font indicator;;
	Font indicator2;;
	Font console;;
	Font verdana;;
	Font verdana_pixel;;
	Font verdana_pixel_bold;;
	Font tahoma;;
	Font tahoma_pixel;;
	Font smallest_pixel_7;
	Font small_fonts;
	Font px_04b03;
	Font manual_antiaim;
	Font arrows;
	Font amogus;
}

void render::init( ) {
	menu       = Font( XOR( "Tahoma" ), 12, FW_NORMAL, FONTFLAG_NONE );
	menu_shade = Font( XOR( "Tahoma" ), 12, FW_NORMAL, FONTFLAG_DROPSHADOW );
	esp        = Font( XOR( "Tahoma" ), 12, FW_BOLD, FONTFLAG_DROPSHADOW );
	esp2        = Font(XOR( "Verdana" ), 12, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	esp_small  = Font(XOR("Smallest Pixel-7"), 10, FW_NORMAL, FONTFLAG_OUTLINE);
	esp_small2 = Font(XOR("Small Fonts"), 8, FW_NORMAL, FONTFLAG_OUTLINE);
	hud        = Font( XOR( "Tahoma" ), 16, FW_NORMAL, FONTFLAG_ANTIALIAS );
	cs         = Font( XOR( "Undefeated" ), 14, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
	indicator  = Font( XOR( "Verdana" ), 26, FW_BOLD, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
	indicator2 = Font(XOR("Bahnschrift"), 28, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	console = Font(XOR("Tahoma"), 12, FW_NORMAL, FONTFLAG_DROPSHADOW);
	verdana = Font(XOR("Verdana"), 12, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	verdana_pixel = Font(XOR("Verdana"), 12, FW_NORMAL, FONTFLAG_DROPSHADOW);
	verdana_pixel_bold = Font(XOR("Verdana"), 12, FW_BOLD, FONTFLAG_DROPSHADOW);
	tahoma = Font(XOR("Tahoma"), 12, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	tahoma_pixel = Font(XOR("Tahoma"), 12, FW_NORMAL, FONTFLAG_DROPSHADOW);
	smallest_pixel_7 = Font(XOR("Smallest Pixel-7"), 11, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE);
	small_fonts = Font(XOR("Small Fonts"), 8, FW_NORMAL, FONTFLAG_OUTLINE);
	px_04b03 = Font(XOR("04B03"), 8, FW_NORMAL, FONTFLAG_OUTLINE);
	manual_antiaim = Font(XOR("Arrows tfb"), 15, FW_NORMAL, FONTFLAG_ANTIALIAS);
	arrows = Font(XOR("Verdana"), 38, FW_THIN, FONTFLAG_ANTIALIAS);
	amogus = Font(XOR("Among Us Regular"), 55, FW_NORMAL, FONTFLAG_ANTIALIAS);
}

bool render::WorldToScreen( const vec3_t& world, vec2_t& screen ) {
	float w;

	const VMatrix& matrix = g_csgo.m_engine->WorldToScreenMatrix( );

	// check if it's in view first.
    // note - dex; w is below 0 when world position is around -90 / +90 from the player's camera on the y axis.
	w = matrix[ 3 ][ 0 ] * world.x + matrix[ 3 ][ 1 ] * world.y + matrix[ 3 ][ 2 ] * world.z + matrix[ 3 ][ 3 ];
	if( w < 0.001f )
		return false;

	// calculate x and y.
	screen.x = matrix[ 0 ][ 0 ] * world.x + matrix[ 0 ][ 1 ] * world.y + matrix[ 0 ][ 2 ] * world.z + matrix[ 0 ][ 3 ];
	screen.y = matrix[ 1 ][ 0 ] * world.x + matrix[ 1 ][ 1 ] * world.y + matrix[ 1 ][ 2 ] * world.z + matrix[ 1 ][ 3 ];

	screen /= w;

	// calculate screen position.
	screen.x = ( g_cl.m_width / 2 ) + ( screen.x * g_cl.m_width ) / 2;
	screen.y = ( g_cl.m_height / 2 ) - ( screen.y * g_cl.m_height ) / 2;

	return true;
}

void render::RoundedBoxStatic(int x, int y, int w, int h, int radius, Color col)
{
	Vertex round[64];

	for (int i = 0; i < 4; i++)
	{
		int _x = x + ((i < 2) ? (w - radius) : radius);
		int _y = y + ((i % 3) ? (h - radius) : radius);

		float a = 90.f * i;

		for (int j = 0; j < 16; j++)
		{
			float _a = DEG2RAD(a + j * 6.f);

			round[(i * 16) + j] = Vertex(vec2_t(_x + radius * sin(_a), _y - radius * cos(_a)));
		}
	}
	g_csgo.m_surface->DrawSetColor(col);
	g_csgo.m_surface->DrawTexturedPolygon(64, round);
}

void render::RoundedBoxStaticOutline(int x, int y, int w, int h, int radius, Color col)
{
	Vertex round[64];

	for (int i = 0; i < 4; i++)
	{
		int _x = x + ((i < 2) ? (w - radius) : radius);
		int _y = y + ((i % 3) ? (h - radius) : radius);

		float a = 90.f * i;

		for (int j = 0; j < 16; j++)
		{
			float _a = DEG2RAD(a + j * 6.f);

			round[(i * 16) + j] = Vertex(vec2_t(_x + radius * sin(_a), _y - radius * cos(_a)));
		}
	}
	g_csgo.m_surface->DrawSetColor(col);
	g_csgo.m_surface->DrawTexturedPolyLine(64, round);
}

void render::WorldCircleOutline(vec3_t origin, float radius, float angle, Color color) {
	std::vector< Vertex > vertices{};

	float step = (1.f / radius) + math::deg_to_rad(angle);

	float lat = 1.f;
	vertices.clear();

	for (float lon{}; lon < math::pi_2; lon += step) {
		vec3_t point{
			origin.x + (radius * std::sin(lat) * std::cos(lon)),
			origin.y + (radius * std::sin(lat) * std::sin(lon)),
			origin.z + (radius * std::cos(lat) - (radius / 2))
		};

		vec2_t screen;
		if (WorldToScreen(point, screen))
			vertices.emplace_back(screen);
	}
	static int texture = g_csgo.m_surface->CreateNewTextureID(false);

	g_csgo.m_surface->DrawSetTextureRGBA(texture, &colors::white, 1, 1);
	g_csgo.m_surface->DrawSetColor(color);

	//g_csgo.m_surface->DrawSetTexture(texture);
	//g_csgo.m_surface->DrawTexturedPolygon(vertices.size(), vertices.data());

	g_csgo.m_surface->DrawTexturedPolyLine(vertices.size(), vertices.data());
}

void render::round_rect(int x, int y, int w, int h, int r, Color color) {
	Vertex round[64];

	for (int i = 0; i < 4; i++) {
		int _x = x + ((i < 2) ? (w - r) : r);
		int _y = y + ((i % 3) ? (h - r) : r);

		float a = 90.f * i;

		for (int j = 0; j < 16; j++) {
			float _a = math::deg_to_rad(a + j * 6.f);

			round[(i * 16) + j] = Vertex(vec2_t(_x + r * sin(_a), _y - r * cos(_a)));
		}
	}

	g_csgo.m_surface->DrawSetColor(color);
	g_csgo.m_surface->DrawTexturedPolygon(64, round);
}

void render::line( vec2_t v0, vec2_t v1, Color color ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawLine( v0.x, v0.y, v1.x, v1.y );
}

void render::line( int x0, int y0, int x1, int y1, Color color ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawLine( x0, y0, x1, y1 );
}

void render::rect( int x, int y, int w, int h, Color color ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawOutlinedRect( x, y, x + w, y + h );
}

void render::rect_filled( int x, int y, int w, int h, Color color ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawFilledRect( x, y, x + w, y + h );
}

void render::RectFilled(vec2_t pos, vec2_t size, Color color) {
	render::rect_filled(pos.x, pos.y, size.x, size.y, color);
}

void render::rect_filled_fade( int x, int y, int w, int h, Color color, int a1, int a2 ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawFilledRectFade( x, y, x + w, y + h, a1, a2, false );
}

void render::rect_outlined( int x, int y, int w, int h, Color color, Color color2 ) {
	rect( x, y, w, h, color );
	rect( x - 1, y - 1, w + 2, h + 2, color2 );
	rect( x + 1, y + 1, w - 2, h - 2, color2 );
}

void render::circle( int x, int y, int radius, int segments, Color color ) {
	static int texture = g_csgo.m_surface->CreateNewTextureID( true );

	g_csgo.m_surface->DrawSetTextureRGBA( texture, &colors::white, 1, 1 );
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawSetTexture( texture );

	std::vector< Vertex > vertices{};

	float step = math::pi_2 / segments;
	for( float i{ 0.f }; i < math::pi_2; i += step )
		vertices.emplace_back( vec2_t{ x + ( radius * std::cos( i ) ), y + ( radius * std::sin( i ) ) } );

	g_csgo.m_surface->DrawTexturedPolygon( vertices.size( ), vertices.data( ) );
}

void render::gradient( int x, int y, int w, int h, Color color1, Color color2 ) {
	g_csgo.m_surface->DrawSetColor( color1 );
	g_csgo.m_surface->DrawFilledRectFade( x, y, x + w, y + h, color1.a( ), 0, false );

	g_csgo.m_surface->DrawSetColor( color2 );
	g_csgo.m_surface->DrawFilledRectFade( x, y, x + w, y + h, 0, color2.a( ), false );
}

void render::gradient2(int x, int y, int w, int h, Color color1, Color color2) {
	g_csgo.m_surface->DrawSetColor(color1);
	g_csgo.m_surface->DrawFilledRectFade(x, y, x + w, y + h, color1.a(), 0, false);

	g_csgo.m_surface->DrawSetColor(color2);
	g_csgo.m_surface->DrawFilledRectFade(x, y, x + w, y + h, 0, color2.a(), false);
}

void render::sphere( vec3_t origin, float radius, float angle, float scale, Color color ) {
	std::vector< Vertex > vertices{};

	// compute angle step for input radius and precision.
	float step = ( 1.f / radius ) + math::deg_to_rad( angle );

	for( float lat{}; lat < ( math::pi * scale ); lat += step ) {
		// reset.
		vertices.clear( );

		for( float lon{}; lon < math::pi_2; lon += step ) {
			vec3_t point{
				origin.x + ( radius * std::sin( lat ) * std::cos( lon ) ),
				origin.y + ( radius * std::sin( lat ) * std::sin( lon ) ),
				origin.z + ( radius * std::cos( lat ) )
			};

			vec2_t screen;
			if( WorldToScreen( point, screen ) )
				vertices.emplace_back( screen );
		}

		if( vertices.empty( ) )
			continue;

		g_csgo.m_surface->DrawSetColor( color );
		g_csgo.m_surface->DrawTexturedPolyLine( vertices.size( ), vertices.data( ) );
	}
}

void render::triangle(vec2_t point_one, vec2_t point_two, vec2_t point_three, Color color)
{
	if (!g_csgo.m_surface)
		return;

	Vertex verts[3] = {
		Vertex(point_one),
		Vertex(point_two),
		Vertex(point_three)
	};

	auto surface = g_csgo.m_surface;

	static int texture = surface->CreateNewTextureID(true);
	unsigned char buffer[4] = { 255, 255, 255, 255 };

	surface->DrawSetTextureRGBA(texture, buffer, 1, 1);
	surface->DrawSetColor(color);
	surface->DrawSetTexture(texture);

	surface->DrawTexturedPolygon(3, verts);
}

void render::onetap(vec2_t point_one, vec2_t point_two, vec2_t point_three, Color color, Color outline)
{
	Vertex verts[3] = {
		Vertex(point_one),
		Vertex(point_two),
		Vertex(point_three)
	};

	static int texture = g_csgo.m_surface->CreateNewTextureID(true);
	unsigned char buffer[4] = { 255, 255, 255, 255 };

	g_csgo.m_surface->DrawSetTextureRGBA(texture, buffer, 1, 1);

	g_csgo.m_surface->DrawSetColor(color);
	g_csgo.m_surface->DrawSetTexture(texture);

	g_csgo.m_surface->DrawTexturedPolygon(3, verts);

	g_csgo.m_surface->DrawSetColor(outline); // filled
	g_csgo.m_surface->DrawTexturedPolyLine(3, verts); // outline
}

bool render::WorldToScreen2(const vec3_t& world, vec3_t& screen) {
	float w;

	const VMatrix& matrix = g_csgo.m_engine->WorldToScreenMatrix();

	// check if it's in view first.
	// note - dex; w is below 0 when world position is around -90 / +90 from the player's camera on the y axis.
	w = matrix[3][0] * world.x + matrix[3][1] * world.y + matrix[3][2] * world.z + matrix[3][3];
	if (w < 0.001f)
		return false;

	// calculate x and y.
	screen.x = matrix[0][0] * world.x + matrix[0][1] * world.y + matrix[0][2] * world.z + matrix[0][3];
	screen.y = matrix[1][0] * world.x + matrix[1][1] * world.y + matrix[1][2] * world.z + matrix[1][3];

	screen /= w;

	// calculate screen position.
	screen.x = (g_cl.m_width / 2) + (screen.x * g_cl.m_width) / 2;
	screen.y = (g_cl.m_height / 2) - (screen.y * g_cl.m_height) / 2;

	return true;
}
#define ZERO vec3_t(0.0f, 0.0f, 0.0f)
void render::Draw3DFilledCircle(const vec3_t& origin, float radius, Color color)
{
	static auto prevScreenPos = ZERO; //-V656
	static auto step = M_PI * 2.0f / 72.0f;

	auto screenPos = ZERO;
	auto screen = ZERO;

	if (!render::WorldToScreen2(origin, screen))
		return;

	for (auto rotation = 0.0f; rotation <= M_PI * 2.0f; rotation += step) //-V1034
	{
		vec3_t pos(radius * cos(rotation) + origin.x, radius * sin(rotation) + origin.y, origin.z);

		if (render::WorldToScreen2(pos, screenPos))
		{
			if (!prevScreenPos.IsZero() && prevScreenPos.IsValid() && screenPos.IsValid() && prevScreenPos != screenPos)
			{
				line(prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color);
				triangle(vec2_t(screen.x, screen.y), vec2_t(screenPos.x, screenPos.y), vec2_t(prevScreenPos.x, prevScreenPos.y), Color(color.r(), color.g(), color.b(), 50));
			}

			prevScreenPos = screenPos;
		}
	}
}

Vertex render::RotateVertex( const vec2_t& p, const Vertex& v, float angle ) {
	// convert theta angle to sine and cosine representations.
	float c = std::cos( math::deg_to_rad( angle ) );
	float s = std::sin( math::deg_to_rad( angle ) );

	return {
        p.x + ( v.m_pos.x - p.x ) * c - ( v.m_pos.y - p.y ) * s, 
        p.y + ( v.m_pos.x - p.x ) * s + ( v.m_pos.y - p.y ) * c 
    };
}

void render::Font::string( int x, int y, Color color, const std::string& text,StringFlags_t flags /*= render::ALIGN_LEFT */ ) {
	wstring( x, y, color, util::MultiByteToWide( text ), flags );
}

void render::Font::string( int x, int y, Color color, const std::stringstream& text, StringFlags_t flags /*= render::ALIGN_LEFT */ ) {
	wstring( x, y, color, util::MultiByteToWide( text.str( ) ), flags );
}

void render::Font::wstring( int x, int y, Color color, const std::wstring& text, StringFlags_t flags /*= render::ALIGN_LEFT */ ) {
	int w, h;

	g_csgo.m_surface->GetTextSize( m_handle, text.c_str( ), w, h );
	g_csgo.m_surface->DrawSetTextFont( m_handle );
	g_csgo.m_surface->DrawSetTextColor( color );

	if( flags & ALIGN_RIGHT )
		x -= w;
	if( flags & render::ALIGN_CENTER )
		x -= w / 2;

	g_csgo.m_surface->DrawSetTextPos( x, y );
	g_csgo.m_surface->DrawPrintText( text.c_str( ), ( int )text.size( ) );
}

// Add this function to render a semi-transparent black background.
void render::DrawSemiTransparentBlackBackground(int x, int y, int w, int h, float alpha) {
	Color backgroundColor(0, 0, 0, static_cast<int>(255 * alpha)); // Black color with specified alpha value
	rect_filled(x, y, w, h, backgroundColor);
}

render::FontSize_t render::Font::size( const std::string& text ) {
	return wsize( util::MultiByteToWide( text ) );
}

render::FontSize_t render::Font::wsize( const std::wstring& text ) {
	FontSize_t res;
	g_csgo.m_surface->GetTextSize( m_handle, text.data( ), res.m_width, res.m_height );
	return res;
}

vec2_t render::GetScreenSize() {
	return vec2_t(g_cl.m_width, g_cl.m_height);
}

void render::flatSphere(vec3_t origin, float radius, float angle, float scale, Color color) {
	std::vector< Vertex > vertices{};

	// compute angle step for input radius and precision.
	float step = (1.f / radius) + math::deg_to_rad(angle);

	for (float lat{}; lat < (math::pi * scale); lat += step) {
		// reset.
		vertices.clear();

		for (float lon{}; lon < math::pi_2; lon += step) {
			// Adjust the scaling of the z-coordinate to make the sphere appear more flattened.
			vec3_t point{
				origin.x + (radius * std::sin(lat) * std::cos(lon)),
				origin.y + (radius * std::sin(lat) * std::sin(lon)),
				origin.z + (0.2f * radius * std::cos(lat)) // Adjust the scale factor (e.g., 0.5)
			};

			vec2_t screen;
			if (WorldToScreen(point, screen))
				vertices.emplace_back(screen);
		}

		if (vertices.empty())
			continue;

		g_csgo.m_surface->DrawSetColor(color);
		g_csgo.m_surface->DrawTexturedPolyLine(vertices.size(), vertices.data());
	}
}