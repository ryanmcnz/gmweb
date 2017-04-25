#define gmweb_start
///gmweb_start()
// Initializes the GMWeb dll

//Configure the initial surface, including its vertex and matrix
global.__gmweb_surface_set = false;
var vb_format, width, height;
global.__gmweb_matrix_identity[15] = 1; global.__gmweb_matrix_identity[10] = 1; global.__gmweb_matrix_identity[5] = 1; global.__gmweb_matrix_identity[0] = 1;
global.__gmweb_vb_quad = vertex_create_buffer();
vertex_format_begin(); vertex_format_add_position(); vertex_format_add_textcoord(); vb_format = vertex_format_end();
vertex_begin(global.__gmweb_vb_quad, vb_format);
    width = 1; height = 1;
    vertex_position(global.__gmweb_vb_quad, 0, 0); vertex_texcoord(global.__gmweb_vb_quad, 0, 0);
    vertex_position(global.__gmweb_vb_quad, width, 0); vertex_texcoord(global.__gmweb_vb_quad, 1, 0);
    vertex_position(global.__gmweb_vb_quad, 0, height); vertex_texcoord(global.__gmweb_vb_quad, 0, 1);
    vertex_position(global.__gmweb_vb_quad, width, 0); vertex_texcoord(global.__gmweb_vb_quad, 1, 0);
    vertex_position(global.__gmweb_vb_quad, 0, height); vertex_texcoord(global.__gmweb_vb_quad, 0, 1);
    vertex_position(global.__gmweb_vb_quad, width, height); vertex_texcoord(global.__gmweb_vb_quad, 1, 1);
vertex_end(global.__gmweb_vb_quad);
global.__gmweb_stack_matrix = ds_stack_create();

// Pass the window handle to gmweb for use with callback events.
//__gmweb_set_window_handle(window_handle());

// Call the native function to start gmweb.
return __gmweb_start(window_device());

#define gmweb_surface_create
/// gmweb_surface_create(width, height, format, D3DUsage = RENDERTARGET)
// Creates a DX surface. It's similar to a normal GameMaker surface, with an important difference being that you can specify a format. Returns DX surface ID.
// width: The width of the DX surface.
// height: The height of the DX surface.
// format: The format of the DX surface. Normal GameMaker surfaces use FORMAT.A8R8G8B8. You can choose a format from the FORMAT enum below.

enum FORMAT {
    UNKNOWN = 0,
    R8G8B8 = 20,
    A8R8G8B8 = 21,
    X8R8G8B8 = 22,
    R5G6B5 = 23,
    X1R5G5B5 = 24,
    A1R5G5B5 = 25,
    A4R4G4B4 = 26,
    R3G3B2 = 27,
    A8 = 28,
    A8R3G3B2 = 29,
    X4R4G4B4 = 30,
    A2B10G10R10 = 31,
    A8B8G8R8 = 32,
    X8B8G8R8 = 33,
    G16R16 = 34,
    A2R10G10B10 = 35,
    A16B16G16R16 = 36,
    A8P8 = 40,
    P8 = 41,
    L8 = 50,
    A8L8 = 51,
    A4L4 = 52,
    V8U8 = 60,
    L6V5U5 = 61,
    X8L8V8U8 = 62,
    Q8W8V8U8 = 63,
    V16U16 = 64,
    A2W10V10U10 = 67,
    UYVY = 1498831189,
    R8G8_B8G8 = 1195525970,
    YUY2 = 844715353,
    G8R8_G8B8 = 111970375,
    DXT1 = 827611204,
    DXT2 = 844388420,
    DXT3 = 861165636,
    DXT4 = 877942852,
    DXT5 = 894720068,
    D16_LOCKABLE = 70,
    D32 = 71,
    D15S1 = 73,
    D24S8 = 75,
    D24X8 = 77,
    D24X4S4 = 79,
    D16 = 80,
    D32F_LOCKABLE = 82,
    D24FS8 = 83,
    L16 = 81,
    VERTEXDATA = 100,
    INDEX16 = 101,
    INDEX32 = 102,
    Q16W16V16U16 = 110,
    MULTI2_ARGB8 = 827606349,
    R16F = 111,
    G16R16F = 112,
    A16B16G16R16F = 113,
    R32F = 114,
    G32R32F = 115,
    A32B32G32R32F = 116,
    CxV8U8 = 117,
    FORCE_DWORD = 2147483647
}

enum D3DUSAGE {
    RENDERTARGET = $00000001,
    DEPTHSTENCIL = $00000002,
    DYNAMIC = $00000200
}

// Allow writing to the surface later. (Required for Awesomium rendering).
//var usage = D3DUSAGE.RENDERTARGET;
//if (argument_count > 3) {
//    usage = argument[3];
//}
// TODO: Allow D3D Usage to be passed in.
var gmweb_sf = __gmweb_surface_create(argument0, argument1, argument2);
var view = global.__gmweb_matrix_identity;

// Cache the surface dimensions for future calls such as rendering.
// Note: It may not be suitable to use room_width and room_height if they can change?
global.__gmweb_surface_width[gmweb_sf] = argument0;
global.__gmweb_surface_height[gmweb_sf] = argument1;
view[0] = argument0; view[5] = argument1; view[12] = room_width; view[13] = room_height;
global.__gmweb_surface_matrix_view[gmweb_sf] = view;

return gmweb_sf;

#define gmweb_draw_surface
/// gmweb_draw_surface(surface_id, x, y)
// Draws a DX surface.
// DX surface ID: The ID of the DX surface to draw.
// x, y: The upper left corner of the DX surface.

var world, world_previous, shader_is_set = gmweb_shader_is_set();

if (!shader_is_set) shader_set(sh_gmweb_draw_surface);
gmweb_surface_set_texture(1, argument0);
world_previous = matrix_get(matrix_world);
world = global.__gmweb_matrix_identity;
world[0] = global.__gmweb_surface_width[argument0]; world[5] = global.__gmweb_surface_height[argument0];
world[12] = argument1; world[13] = argument2;
matrix_set(matrix_world, world);
vertex_submit(global.__gmweb_vb_quad, pr_trianglelist, -1);
matrix_set(matrix_world, world_previous);
if (!shader_is_set) shader_reset();

//return;

#define gmweb_surface_reset_target
/// gmweb_surface_reset_target()
// Resets the target.

matrix_set(matrix_projection, ds_stack_pop(global.__gmweb_stack_matrix));
matrix_set(matrix_view, ds_stack_pop(global.__gmweb_stack_matrix));
matrix_set(matrix_world, ds_stack_pop(global.__gmweb_stack_matrix));

__gmweb_surface_reset_target();

global.__gmweb_surface_set = false;

//return;

#define gmweb_surface_set_target
/// gmweb_surface_set_target(index, DX surface ID)
// Sets a DX surface as a target.
// index: The render target index to use. This is normally 0, unless you have multiple render targets.
// DX surface ID: The ID of the DX surface to set as a target.

global.__gmweb_surface_set = true;

ds_stack_push(global.__gmweb_stack_matrix, matrix_get(matrix_world), matrix_get(matrix_view), matrix_get(matrix_projection));

matrix_set(matrix_world, global.__gmweb_matrix_identity);
matrix_set(matrix_view, global.__gmweb_surface_matrix_view[argument1]);
matrix_set(matrix_projection, global.__gmweb_matrix_identity);

return __gmweb_surface_set_target(argument0, argument1);