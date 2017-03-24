/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 09.02.2016
 *
 * Shared system functions
 *
 */

// TOP

#include "4ed_system_shared.h"
#include "4ed_font_interface.h"
#include "4ed_font_interface_to_os.h"
#include "4ed_font_data.h"

struct Win32_Fonts{
    Partition part;
    Render_Font fonts[32];
    u32 font_count;
};

global Win32_Fonts win32_fonts = {0};

internal
Sys_Font_Get_Count_Sig(system_font_get_count){
    return(5);
}

internal
Sys_Font_Get_IDs_By_Index_Sig(system_font_get_ids_by_index){
    b32 result = false;
    u32 stop_index = first_index + index_count;
    if (stop_index <= win32_fonts.font_count){
        result = true;
        for (u32 i = first_index; i < stop_index; ++i){
            id_out[i-first_index] = i;
        }
    }
    return(result);
}

internal
Sys_Font_Get_Name_By_Index_Sig(system_font_get_name_by_index){
    u32 length = 0;
    if (font_index < win32_fonts.font_count){
        Render_Font *font = &win32_fonts.fonts[font_index];
        char *name = font->name;
        length = font->name_len;
        copy_partial_cs(str_out, str_out_cap, make_string(name, length));
    }
    return(length);
}

internal
Sys_Font_Get_Name_By_ID_Sig(system_font_get_name_by_id){
    u32 font_index = font_id;
    u32 result = system_font_get_name_by_index(font_index, str_out, str_out_cap);
    return(result);
}

internal
Sys_Font_Get_Render_Data_By_ID_Sig(system_font_get_render_data_by_id){
    Render_Font *result = 0;
    u32 font_index = font_id;
    if (font_index < win32_fonts.font_count){
        result = &win32_fonts.fonts[font_index];
    }
    return(result);
}

internal
Sys_Font_Load_Page_Sig(system_font_load_page){
    system_set_page(&win32vars.system, &win32_fonts.part, font, page, page_number, 16, true);
}

internal
Sys_Font_Allocate_Sig(system_font_allocate){
    void *result = system_memory_allocate(size);
    return(result);
}

internal
Sys_Font_Free_Sig(system_font_free){
    system_memory_free(ptr, 0);
}

internal
Sys_Font_Init_Sig(system_font_init){
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
    font->get_count = system_font_get_count;
    font->get_ids_by_index = system_font_get_ids_by_index;
    font->get_name_by_index = system_font_get_name_by_index;
    font->get_name_by_id = system_font_get_name_by_id;
    font->get_render_data_by_id = system_font_get_render_data_by_id;
    font->load_page = system_font_load_page;
    font->allocate = system_font_allocate;
    font->free = system_font_free;
    
    font_size = clamp_bottom(8, font_size);
    
    struct TEST_DATA{
        char *c_filename;
        i32 filename_len;
        char *c_name;
        i32 name_len;
    };
    TEST_DATA TEST_SETUP[] = {
        {literal("fonts/LiberationSans-Regular.ttf"), literal("Liberation Sans"), },
        {literal("fonts/liberation-mono.ttf"),        literal("Liberation Mono"), },
        {literal("fonts/Hack-Regular.ttf"),           literal("Hack"),            },
        {literal("fonts/CutiveMono-Regular.ttf"),     literal("Cutive Mono"),     },
        {literal("fonts/Inconsolata-Regular.ttf"),    literal("Inconsolata"),     },
    };
    
    struct Font_Setup{
        Font_Setup *next_font;
        char *c_filename;
        char *c_name;
        i32 filename_len;
        i32 name_len;
    };
    
    Font_Setup *first_setup = 0;
    Font_Setup *head_setup = 0;
    
    u32 TEST_COUNT = ArrayCount(TEST_SETUP);
    for (u32 i = 0; i < TEST_COUNT; ++i){
        if (first_setup == 0){
            head_setup = push_struct(scratch, Font_Setup);
            first_setup = head_setup;
        }
        else{
            head_setup->next_font = push_struct(scratch, Font_Setup);
            head_setup = head_setup->next_font;
        }
        
        TEST_DATA *TEST = &TEST_SETUP[i];
        
        head_setup->c_filename = push_array(scratch, char, TEST->filename_len+1);
        memcpy(head_setup->c_filename, TEST->c_filename, TEST->filename_len+1);
        head_setup->filename_len = TEST->filename_len;
        
        head_setup->c_name = push_array(scratch, char, TEST->name_len+1);
        memcpy(head_setup->c_name, TEST->c_name, TEST->name_len+1);
        head_setup->name_len = TEST->name_len;
        
        partition_align(scratch, 8);
    }
    
    u32 font_count_max = ArrayCount(win32_fonts.fonts);
    u32 font_count = 0;
    u32 i = 0;
    for (Font_Setup *ptr = first_setup; ptr != 0; ptr = ptr->next_font, ++i){
        if (i < font_count_max){
            String filename = make_string(ptr->c_filename, ptr->filename_len);
            String name = make_string(ptr->c_name, ptr->name_len);
            Render_Font *render_font = &win32_fonts.fonts[i];
            
            char full_filename_space[256];
            String full_filename = make_fixed_width_string(full_filename_space);
            sysshared_to_binary_path(&full_filename, filename.str);
            
            system_set_font(&win32vars.system, &win32_fonts.part, render_font, full_filename, name, font_size, use_hinting);
        }
        
        ++font_count;
    }
    
    win32_fonts.font_count = clamp_top(font_count, font_count_max);
    
    end_temp_memory(temp);
}

// BOTTOM

