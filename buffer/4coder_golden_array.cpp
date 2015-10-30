/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 16.10.2015
 * 
 * Buffer data object
 *  type - Golden Array
 * 
 */

// TOP

typedef struct{
    char *data;
    int size, max;
    
    int *line_starts;
    float *line_widths;
    int line_count;
    int line_max;
    int widths_max;
} Buffer;

inline_4tech int
buffer_size(Buffer *buffer){
    return buffer->size;
}

internal_4tech void
buffer_initialize(Buffer *buffer, char *data, int size){
    assert_4tech(size <= buffer->max);
    buffer->size = eol_convert_in(buffer->data, data, size);
}

internal_4tech void*
buffer_relocate(Buffer *buffer, char *new_data, int new_max){
    void *result;
    
    assert_4tech(new_max >= buffer->size);
    
    result = buffer->data;
    memcpy_4tech(new_data, buffer->data, buffer->size);
    buffer->data = new_data;
    buffer->max = new_max;
    
    return(result);
}

typedef struct{
    Buffer *buffer;
    char *data, *end;
    int absolute_pos;
    int size;
    int page_size;
} Buffer_Stringify_Loop;

inline_4tech Buffer_Stringify_Loop
buffer_stringify_loop(Buffer *buffer, int start, int end, int page_size){
    Buffer_Stringify_Loop result;
    if (0 <= start && start < end && end <= buffer->size){
        result.buffer = buffer;
        result.absolute_pos = start;
        result.data = buffer->data + start;
        result.size = end - start;
        result.end = buffer->data + end;
        result.page_size = page_size;
        if (result.size > page_size) result.size = page_size;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_stringify_good(Buffer_Stringify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

inline_4tech void
buffer_stringify_next(Buffer_Stringify_Loop *loop){
    if (loop->data + loop->size == loop->end) loop->buffer = 0;
    else{
        loop->data += loop->page_size;
        loop->absolute_pos += loop->page_size;
        loop->size = (int)(loop->end - loop->data);
        if (loop->size > loop->page_size) loop->size = loop->page_size;
    }
}

typedef struct{
    Buffer *buffer;
    char *data, *end;
    int absolute_pos;
    int size;
    int page_size;
} Buffer_Backify_Loop;

inline_4tech Buffer_Backify_Loop
buffer_backify_loop(Buffer *buffer, int start, int end, int page_size){
    Buffer_Backify_Loop result;
    
    ++start;
    if (0 <= end && end < start && start <= buffer->size){
        result.buffer = buffer;
        result.end = buffer->data + end;
        result.page_size = page_size;
        result.size = start - end;
        if (result.size > page_size) result.size = page_size;
        result.absolute_pos = start - result.size;
        result.data = buffer->data + result.absolute_pos;
    }
    else result.buffer = 0;
    return(result);
}

inline_4tech int
buffer_backify_good(Buffer_Backify_Loop *loop){
    int result;
    result = (loop->buffer != 0);
    return(result);
}

inline_4tech void
buffer_backify_next(Buffer_Backify_Loop *loop){
    char *old_data;
    if (loop->data == loop->end) loop->buffer = 0;
    else{
        old_data = loop->data;
        loop->data -= loop->page_size;
        loop->absolute_pos -= loop->page_size;
        if (loop->data < loop->end){
            loop->size = (int)(old_data - loop->end);
            loop->data = loop->end;
            loop->absolute_pos = 0;
        }
    }
}

internal_4tech int
buffer_replace_range(Buffer *buffer, int start, int end, char *str, int len, int *shift_amount){
    char *data;
    int result;
    int size;
    
    size = buffer_size(buffer);
    assert_4tech(0 <= start);
    assert_4tech(start <= end);
    assert_4tech(end <= size);
    
    *shift_amount = (len - (end - start));
    if (*shift_amount + size <= buffer->max){
        data = (char*)buffer->data;
        memmove_4tech(data + end + *shift_amount, data + end, buffer->size - end);
        buffer->size += *shift_amount;
        data[buffer->size] = 0;
        if (len != 0) memcpy_4tech(data + start, str, len);
        
        result = 0;
    }
    else{
        result = *shift_amount + size;
    }
    
    return(result);
}

internal_4tech int
buffer_batch_edit_step(Buffer_Batch_State *state, Buffer *buffer, Buffer_Edit *sorted_edits, char *strings, int edit_count){
    Buffer_Edit *edit;
    int i, result;
    int shift_total, shift_amount;
    
    result = 0;
    shift_total = state->shift_total;
    i = state->i;
    
    edit = sorted_edits + i;
    for (; i < edit_count; ++i, ++edit){
        result = buffer_replace_range(buffer, edit->start + shift_total, edit->end + shift_total,
                                      strings + edit->str_start, edit->len, &shift_amount);
        if (result) break;
        shift_total += shift_amount;
    }
    
    state->shift_total = shift_total;
    state->i = i;
    
    return(result);
}

#if 0
internal_4tech int
buffer_find_hard_start(Buffer *buffer, int line_start, int *all_whitespace, int *all_space,
                       int *preferred_indent, int tab_width){
    char *data;
    int size;
    int result;
    char c;
    
    *all_space = 1;
    *preferred_indent = 0;
    
    data = buffer->data;
    size = buffer->size;
    
    tab_width -= 1;
    
    for (result = line_start; result < size; ++result){
        c = data[result];
        if (c == '\n' || c == 0){
            *all_whitespace = 1;
            break;
        }
        if (c >= '!' && c <= '~') break;
        if (c == '\t') *preferred_indent += tab_width;
        if (c != ' ') *all_space = 0;
        *preferred_indent += 1;
    }
    
    return(result);
}
#endif

// BOTTOM

