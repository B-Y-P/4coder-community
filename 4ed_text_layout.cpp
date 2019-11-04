/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 31.03.2019
 *
 * Text layout representation
 *
 */

// TOP

internal void
text_layout_init(Thread_Context *tctx, Text_Layout_Container *container){
    block_zero_struct(container);
    container->node_arena = reserve_arena(tctx);
    container->table = make_table_u64_u64(tctx->allocator, 20);
}

internal Text_Layout*
text_layout_new__alloc_layout(Text_Layout_Container *container){
    Text_Layout *node = container->free_nodes;
    if (node == 0){
        node = push_array(container->node_arena, Text_Layout, 1);
    }
    else{
        sll_stack_pop(container->free_nodes);
    }
    return(node);
}

internal void
text_layout_release(Thread_Context *tctx, Models *models, Text_Layout_Container *container, Text_Layout *layout){
    release_arena(tctx, layout->arena);
    sll_stack_push(container->free_nodes, layout);
}

internal Text_Layout_ID
text_layout_new(Text_Layout_Container *container, Arena *arena,
                Buffer_ID buffer_id, Buffer_Point point,
                Range_i64 visible_range, Interval_i64 visible_line_number_range,
                Rect_f32 rect, FColor *item_colors, Layout_Function *layout_func){
    Text_Layout *new_layout_data = text_layout_new__alloc_layout(container);
    new_layout_data->arena = arena;
    new_layout_data->buffer_id = buffer_id;
    new_layout_data->point = point;
    new_layout_data->visible_range = visible_range;
    new_layout_data->visible_line_number_range = visible_line_number_range;
    new_layout_data->rect = rect;
    new_layout_data->item_colors = item_colors;
    new_layout_data->layout_func = layout_func;
    Text_Layout_ID new_id = ++container->id_counter;
    table_insert(&container->table, new_id, (u64)PtrAsInt(new_layout_data));
    return(new_id);
}

internal Text_Layout*
text_layout_get(Text_Layout_Container *container, Text_Layout_ID id){
    Text_Layout *result = 0;
    Table_Lookup lookup = table_lookup(&container->table, id);
    if (lookup.found_match){
        u64 ptr_val = 0;
        table_read(&container->table, lookup, &ptr_val);
        result = (Text_Layout*)IntAsPtr(ptr_val);
    }
    return(result);
}

internal b32
text_layout_erase(Thread_Context *tctx, Models *models, Text_Layout_Container *container, Text_Layout_ID id){
    b32 result = false;
    Table_Lookup lookup = table_lookup(&container->table, id);
    if (lookup.found_match){
        u64 ptr_val = 0;
        table_read(&container->table, lookup, &ptr_val);
        Text_Layout *ptr = (Text_Layout*)IntAsPtr(ptr_val);
        text_layout_release(tctx, models, container, ptr);
        table_erase(&container->table, lookup);
        result = true;
    }
    return(result);
}

////////////////////////////////

internal void
text_layout_render(Thread_Context *tctx, Models *models, Text_Layout *layout){
    Editing_File *file = imp_get_file(models, layout->buffer_id);
    if (file != 0){
        Render_Target *target = models->target;
        Color_Table color_table = models->color_table;
        Face *face = file_get_face(models, file);
        f32 width = rect_width(layout->rect);
        
        ARGB_Color special_color = color_table.vals[Stag_Special_Character];
        ARGB_Color ghost_color   = color_table.vals[Stag_Ghost_Character];
        
        Vec2_f32 shift_p = layout->rect.p0 - layout->point.pixel_shift;
        i64 first_index = layout->visible_range.first;
        i64 line_number = layout->visible_line_number_range.min;
        i64 line_number_last = layout->visible_line_number_range.max;
        Layout_Function *layout_func = layout->layout_func;
        for (;line_number <= line_number_last; line_number += 1){
            Layout_Item_List line = file_get_line_layout(tctx, models, file,
                                                         layout_func, width, face,
                                                         line_number);
            for (Layout_Item_Block *block = line.first;
                 block != 0;
                 block = block->next){
                Layout_Item *item = block->items;
                i64 count = block->item_count;
                FColor *item_colors = layout->item_colors;
                for (i32 i = 0; i < count; i += 1, item += 1){
                    if (item->codepoint != 0){
                        ARGB_Color color = 0;
                        if (HasFlag(item->flags, LayoutItemFlag_Special_Character)){
                            color = special_color;
                        }
                        else if (HasFlag(item->flags, LayoutItemFlag_Ghost_Character)){
                            color = ghost_color;
                        }
                        else{
                            FColor fcolor = item_colors[item->index - first_index];
                            color = finalize_color(color_table, fcolor);
                        }
                        Vec2_f32 p = item->rect.p0 + shift_p;
                        draw_font_glyph(target, face, item->codepoint, 
                                        p, color, GlyphFlag_None);
                    }
                }
            }
            shift_p.y += line.height;
        }
    }
}

// BOTTOM
