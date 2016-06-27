/*
* Mr. 4th Dimention - Allen Webster
*
* 25.02.2016
*
* File editing view for 4coder
*
*/

// TOP

#include "4ed_meta.h"
#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4cpp_types.h"
#include "4cpp_lexer_types.h"

#define FCPP_LEXER_IMPLEMENTATION
#include "4cpp_lexer.h"

#include "4coder_version.h"

struct Struct_Field{
    char *type;
    char *name;
};

void to_lower(char *src, char *dst){
    char *c, ch;
    for (c = src; *c != 0; ++c){
        ch = char_to_lower(*c);
        *dst++ = ch;
    }
    *dst = 0;
}

void to_lower(String *str){
    char *c;
    int i = 0;
    int size = str->size;
    for (c = str->str; i < size; ++c, ++i){
        *c = char_to_lower(*c);
    }
}

void to_upper(char *src, char *dst){
    char *c, ch;
    for (c = src; *c != 0; ++c){
        ch = char_to_upper(*c);
        *dst++ = ch;
    }
    *dst = 0;
}

void to_upper(String *str){
    char *c;
    int i = 0;
    int size = str->size;
    for (c = str->str; i < size; ++c, ++i){
        *c = char_to_upper(*c);
    }
}

void to_camel(char *src, char *dst){
    char *c, ch;
    int is_first = 1;
    for (c = src; *c != 0; ++c){
        ch = *c;
        if (char_is_alpha_numeric_true(ch)){
            if (is_first){
                is_first = 0;
                ch = char_to_upper(ch);
            }
            else{
                ch = char_to_lower(ch);
            }
        }
        else{
            is_first = 1;
        }
        *dst++ = ch;
    }
    *dst = 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void struct_begin(FILE *file, char *name){
    fprintf(file, "struct %s{\n", name);
}

void struct_fields(FILE *file, Struct_Field *fields, int count){
    int i;
    for (i = 0; i < count; ++i){
        fprintf(file, "    %s %s;\n", fields[i].type, fields[i].name);
    }
}

void struct_end(FILE *file){
    fprintf(file, "};\n\n");
}


void enum_begin(FILE *file, char *name){
    fprintf(file, "enum %s{\n", name);
}


char *keys_that_need_codes[] = {
    "back",
    "up",
    "down",
    "left",
    "right",
    "del",
    "insert",
    "home",
    "end",
    "page_up",
    "page_down",
    "esc",
    
    "mouse_left",
    "mouse_right",
    
    "f1",
    "f2",
    "f3",
    "f4",
    "f5",
    "f6",
    "f7",
    "f8",
    
    "f9",
    "f10",
    "f11",
    "f12",
    "f13",
    "f14",
    "f15",
    "f16",
};

char* generate_keycode_enum(){
    FILE *file;
    char *filename = "4coder_keycodes.h";
    int i, count;
    unsigned char code = 1;
    
    file = fopen(filename, "wb");
    fprintf(file, "enum Key_Code{\n");
    count = ArrayCount(keys_that_need_codes);
    for (i = 0; i < count; i){
        if (strcmp(keys_that_need_codes[i], "f1") == 0 && code < 0x7F){
            code = 0x7F;
        }
        switch (code){
            case '\n': code++; break;
            case '\t': code++; break;
            case 0x20: code = 0x7F; break;
            default:
            fprintf(file, "key_%s = %d,\n", keys_that_need_codes[i++], code++);
            break;
        }
    }
    fprintf(file, "};\n");
    
    fprintf(file,
            "static char*\n"
            "global_key_name(int key_code, int *size){\n"
            "char *result = 0;\n"
            "switch(key_code){\n"
            );
    for (i = 0; i < count; ++i){
        fprintf(file,
                "case key_%s: result = \"%s\"; *size = sizeof(\"%s\")-1; break;\n",
                keys_that_need_codes[i],
                keys_that_need_codes[i],
                keys_that_need_codes[i]
                );
    }
    fprintf(file,
            "}\n"
            "return(result);\n"
            "}\n"
            );
    
    fclose(file);
    return(filename);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
char* bar_style_fields[] = {
    "bar",
    "bar_active",
    "base",
    "pop1",
    "pop2",
};

char* main_style_fields[] = {
    "back",
    "margin",
    "margin_hover",
    "margin_active",
    "cursor",
    "at_cursor",
    "highlight",
    "at_highlight",
    "mark",
    "default",
    "comment",
    "keyword",
    "str_constant",
    "char_constant",
    "int_constant",
    "float_constant",
    "bool_constant",
    "preproc",
    "include",
    "special_character",
    "highlight_junk",
    "highlight_white",
    "paste",
    "undo",
    "next_undo",
};

static char*
make_style_tag(char *tag){
    char *str;
    int len;
    
    len = (int)strlen(tag);
    str = (char*)malloc(len + 1);
    to_camel(tag, str);
    str[len] = 0;
    
    return(str);
}

char style_index_function_start[] =
"inline u32*\n"
"style_index_by_tag(Style_Main_Data *s, u32 tag){\n"
" u32 *result = 0;\n"
" switch (tag){\n";

char style_index_function_end[] =
" }\n"
" return(result);\n"
"}\n\n";

char style_case[] = " case Stag_%s: result = &s->%s_color; break;\n";
char style_info_case[] = " case Stag_%s: result = &s->file_info_style.%s_color; break;\n";

char* generate_style(){
    char *filename = "4coder_style.h & 4ed_style.h";
    char filename_4coder[] = "4coder_style.h";
    char filename_4ed[] = "4ed_style.h";
    FILE *file;
    char *tag;
    int count, i;
    
    file = fopen(filename_4coder, "wb");
    enum_begin(file, "Style_Tag");
    {
        count = ArrayCount(bar_style_fields);
        for (i = 0; i < count; ++i){
            tag = make_style_tag(bar_style_fields[i]);
            fprintf(file, "Stag_%s,\n", tag);
            free(tag);
        }
        
        count = ArrayCount(main_style_fields);
        for (i = 0; i < count; ++i){
            tag = make_style_tag(main_style_fields[i]);
            fprintf(file, "Stag_%s,\n", tag);
            free(tag);
        }
    }
    struct_end(file);
    fclose(file);
    
    file = fopen(filename_4ed, "wb");
    struct_begin(file, "Interactive_Style");
    {
        count = ArrayCount(bar_style_fields);
        for (i = 0; i < count; ++i){
            fprintf(file, "u32 %s_color;\n", bar_style_fields[i]);
        }
    }
    struct_end(file);
    
    struct_begin(file, "Style_Main_Data");
    {
        count = ArrayCount(main_style_fields);
        for (i = 0; i < count; ++i){
            fprintf(file, "u32 %s_color;\n", main_style_fields[i]);
        }
        fprintf(file, "Interactive_Style file_info_style;\n");
    }
    struct_end(file);
    
    {
        fprintf(file, "%s", style_index_function_start);
        count = ArrayCount(bar_style_fields);
        for (i = 0; i < count; ++i){
            tag = make_style_tag(bar_style_fields[i]);
            fprintf(file, style_info_case, tag, bar_style_fields[i]);
            free(tag);
        }
        
        count = ArrayCount(main_style_fields);
        for (i = 0; i < count; ++i){
            tag = make_style_tag(main_style_fields[i]);
            fprintf(file, style_case, tag, main_style_fields[i]);
            free(tag);
        }
        fprintf(file, "%s", style_index_function_end);
    }
    
    fclose(file);
    
    return(filename);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct Function_Set{
    String *name;
    String *ret;
    String *args;
    
    String *macros;
    String *public_name;
    String *doc_string;
    
    int    *valid;
} Function_Set;

void
zero_index(Function_Set fnc_set, int sig_count){
    fnc_set.name [sig_count] = string_zero();
    fnc_set.ret  [sig_count] = string_zero();
    fnc_set.args [sig_count] = string_zero();
    fnc_set.valid[sig_count] = 0;
}

String
file_dump(char *filename){
    String result = {0};
    FILE *file = fopen(filename, "rb");
    
    if (file){
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        result.memory_size = result.size + 1;
        result.str = (char*)malloc(result.memory_size);
        
        fread(result.str, 1, result.size, file);
        result.str[result.size] = 0;
        
        fclose(file);
    }
    
    return(result);
}

String
get_first_line(String source){
    String line = {0};
    int pos = find(source, 0, '\n');
    
    line = substr(source, 0, pos);
    
    return(line);
}

String
get_next_line(String source, String line){
    String next = {0};
    int pos = (int)(line.str - source.str) + line.size;
    int start = 0;
    
    if (pos < source.size){
        assert(source.str[pos] == '\n');
        start = pos + 1;
        
        if (start < source.size){
            pos = find(source, start, '\n');
            next = substr(source, start, pos - start);
        }
    }
    
    return(next);
}

String
skip_whitespace(String str){
    String result = {0};
    int i = 0;
    for (; i < str.size && char_is_whitespace(str.str[i]); ++i);
    result = substr(str, i, str.size - i);
    return(result);
}

String
chop_whitespace(String str){
    String result = {0};
    int i = str.size;
    for (; i > 0 && char_is_whitespace(str.str[i-1]); --i);
    result = substr(str, 0, i);
    return(result);
}

int
is_comment(String str){
    int result = 0;
    if (str.size >= 2){
        if (str.str[0] == '/' &&
            str.str[1] == '/'){
            result = 1;
        }
    }
    return(result);
}

struct Parse{
    Cpp_Token_Stack tokens;
};

int
check_and_fix_docs(String *lexeme){
    int result = false;
    
    if (lexeme->size > 4){
        if (lexeme->str[0] == '/'){
            if (lexeme->str[1] == '*'){
                if (lexeme->str[lexeme->size - 2] == '*'){
                    if (lexeme->str[lexeme->size - 1] == '/'){
                        result = true;
                        lexeme->str += 2;
                        lexeme->size -= 4;
                    }
                }
            }
        }
    }
    
    return(result);
}

static String
doc_note_string[] = {
    make_lit_string("DOC_PARAM"),
    make_lit_string("DOC_RETURN"),
    make_lit_string("DOC"),
    make_lit_string("DOC_SEE"),
};

String
doc_parse_identifier(String lexeme, int *pos){
    String result = {0};
    return(result);
}

void
perform_doc_parse(Parse *parse, String lexeme){
    int keep_parsing = true;
    int pos = 0;
    
    do{
        String doc_note = doc_parse_identifier(lexeme, &pos);
        if (doc_note.size == 0){
            keep_parsing = false;
        }
        else{
            int doc_note_type;
            if (string_set_match(doc_note_string, ArrayCount(doc_note_string), doc_note, &doc_note_type)){
                // TODO(allen): switch on the note type and add the info to the parse data
            }
            else{
                // TODO(allen): do warning
            }
        }
    }while(keep_parsing);
}

char*
generate_custom_headers(){
#define API_H "4coder_custom_api.h"
#define API_DOC "4coder_API.html"
    
    char *filename = API_H " & " API_DOC;
    
    Function_Set function_set = {0};
    
#if 0
    // NOTE(allen): Header
    String data = file_dump("custom_api_spec.cpp");
    
    int line_count = 0;
    String line = {0};
    for (line = get_first_line(data);
         line.str;
         line = get_next_line(data, line)){
        ++line_count;
    }
    
    int max_name_size = 0;
    int sig_count = 0;
    line_count = 0;
    for (line = get_first_line(data);
         line.str;
         line = get_next_line(data, line)){
        
        ++line_count;
        
        String parse = line;
        parse = skip_whitespace(parse);
        parse = chop_whitespace(parse);
        if (parse.size > 0){
            if (!is_comment(parse)){
                zero_index(function_set, sig_count);
                int valid = false;
                
                int pos = find(parse, 0, ' ');
                function_set.ret[sig_count] = substr(parse, 0, pos);
                parse = substr(parse, pos);
                parse = skip_whitespace(parse);
                
                if (parse.size > 0){
                    pos = find(parse, 0, '(');
                    
                    String name_string = substr(parse, 0, pos);
                    function_set.name[sig_count] = chop_whitespace(name_string);
                    parse = substr(parse, pos);
                    
                    if (parse.size > 0){
                        char end = parse.str[parse.size - 1];
                        valid = true;
                        
                        switch (end){
                            case ')':
                            function_set.args[sig_count] = parse;
                            break;
                            
                            case ';':
                            --parse.size;
                            function_set.args[sig_count] = parse;
                            break;
                            
                            default:
                            valid = false;
                            break;
                        }
                        function_set.valid[sig_count] = valid;
                        
                        if (max_name_size < name_string.size){
                            max_name_size = name_string.size;
                        }
                    }
                }
                
                if (!valid){
                    printf("custom_api_spec.cpp(%d) : generator warning : invalid function signature\n",
                           line_count);
                }
                
                ++sig_count;
            }
        }
    }
#endif
    
    // NOTE(allen): Documentation
    String code_data[2];
    code_data[0] = file_dump("4ed_api_implementation.cpp");
    code_data[1] = file_dump("win32_api_impl.cpp");
    Parse parses[2];
    
    int max_name_size = 0;
    int line_count = 0;
    
    for (int J = 0; J < 2; ++J){
        String *code = &code_data[J];
        Parse *parse = &parses[J];
        
        // TODO(allen): KILL THIS FUCKIN' Cpp_File FUCKIN' NONSENSE HORSE SHIT!!!!!
        Cpp_File file;
        file.data = code->str;
        file.size = code->size;
        
        parse->tokens = cpp_make_token_stack(512);
        cpp_lex_file(file, &parse->tokens);
        
        int count = parse->tokens.count;
        Cpp_Token *tokens = parse->tokens.tokens;
        
        Cpp_Token *token = tokens;
        
        for (int i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(file.data + token->start, token->size);
                if (match(lexeme, "API_EXPORT")){
                    for (; i < count; ++i, ++token){
                        if (token->type == CPP_TOKEN_PARENTHESE_OPEN){
                            break;
                        }
                    }
                    
                    if (i < count){
                        --i;
                        --token;
                        
                        if (token->type == CPP_TOKEN_IDENTIFIER){
                            ++line_count;
                            
                            if (max_name_size < token->size){
                                max_name_size = token->size;
                            }
                        }
                    }
                }
            }
        }
    }
    
    int total_memory = (sizeof(String)*6 + sizeof(int))*line_count;
    
    function_set.name        = (String*)malloc(total_memory);
    function_set.ret         = function_set.name + line_count;
    function_set.args        = function_set.ret + line_count;
    function_set.macros      = function_set.args + line_count;
    function_set.public_name = function_set.macros + line_count;
    function_set.doc_string  = function_set.public_name + line_count;
    function_set.valid       = (int*)(function_set.doc_string + line_count);
    
    memset(function_set.name, 0, total_memory);
    
    int sig_count = 0;
    for (int J = 0; J < 2; ++J){
        String *code = &code_data[J];
        Parse *parse = &parses[J];
        
        // TODO(allen): KILL THIS FUCKIN' Cpp_File FUCKIN' NONSENSE HORSE SHIT!!!!!
        Cpp_File file;
        file.data = code->str;
        file.size = code->size;
        
        int count = parse->tokens.count;
        Cpp_Token *tokens = parse->tokens.tokens;
        
        Cpp_Token *token = 0;
        
        // NOTE(allen): Header Parse
        token = tokens;
        for (int i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(file.data + token->start, token->size);
                if (match(lexeme, "API_EXPORT")){
                    ++i;
                    ++token;
                    if (i < count){
                        Cpp_Token *ret_start_token = token;
                        
                        for (; i < count; ++i, ++token){
                            if (token->type == CPP_TOKEN_PARENTHESE_OPEN){
                                break;
                            }
                        }
                        
                        Cpp_Token *args_start_token = token;
                        
                        if (i < count){
                            --i;
                            --token;
                            
                            function_set.name[sig_count] = make_string(file.data + token->start, token->size);
                            
                            int size = token->start - ret_start_token->start;
                            String ret = make_string(file.data + ret_start_token->start, size);
                            ret = chop_whitespace(ret);
                            function_set.ret[sig_count] = ret;
                            
                            for (; i < count; ++i, ++token){
                                if (token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                                    break;
                                }
                            }
                            
                            if (i < count){
                                int size = token->start + token->size - args_start_token->start;;
                                function_set.args[sig_count] =
                                    make_string(file.data + args_start_token->start, size);
                                function_set.valid[sig_count] = true;
                            }
                        }
                    }
                    
                    if (!function_set.valid[sig_count]){
                        function_set.ret[sig_count] = string_zero();
                        function_set.name[sig_count] = string_zero();
                        function_set.args[sig_count] = string_zero();
                        // TODO(allen): get warning line numbers
                        printf("custom_api_spec.cpp(???) : generator warning : invalid function signature\n");
                    }
                    ++sig_count;
                }
            }
        }
        
        // NOTE(allen): Documentation Parse
        token = tokens;
        for (int i = 0; i < count; ++i, ++token){
            if (token->type == CPP_TOKEN_IDENTIFIER &&
                !(token->flags & CPP_TFLAG_PP_BODY)){
                String lexeme = make_string(file.data + token->start, token->size);
                if (match(lexeme, "API_EXPORT")){
                    
                    for (; i < count; ++i, ++token){
                        if (token->type == CPP_TOKEN_PARENTHESE_OPEN){
                            break;
                        }
                    }
                    
                    if (i < count){
                        --i;
                        --token;
                        
                        if (token->type == CPP_TOKEN_IDENTIFIER){
                            lexeme = make_string(file.data + token->start, token->size);
                            int match = 0;
                            if (string_set_match(function_set.name, sig_count, lexeme, &match)){
                                for (; i < count; ++i, ++token){
                                    if (token->type == CPP_TOKEN_COMMENT){
                                        lexeme = make_string(file.data + token->start, token->size);
                                        if (check_and_fix_docs(&lexeme)){
                                            function_set.doc_string[match] = lexeme;
                                            perform_doc_parse(parse, lexeme);
                                            break;
                                        }
                                    }
                                    else if (token->type == CPP_TOKEN_BRACE_OPEN){
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < sig_count; ++i){
        String name_string = function_set.name[i];
        String *macro = function_set.macros + i;
        String *public_name = function_set.public_name + i;
        
        macro->size = 0;
        macro->memory_size = name_string.size+4;
        
        macro->str = (char*)malloc(macro->memory_size);
        copy(macro, name_string);
        to_upper(macro);
        append(macro, make_lit_string("_SIG"));
        
        
        public_name->size = 0;
        public_name->memory_size = name_string.size;
        
        public_name->str = (char*)malloc(public_name->memory_size);
        copy(public_name, name_string);
        to_lower(public_name);
    }
    
    // NOTE(allen): Header
    FILE *file = fopen(API_H, "wb");
    
    for (int i = 0; i < sig_count; ++i){
        String ret_string   = function_set.ret[i];
        String args_string  = function_set.args[i];
        String macro_string = function_set.macros[i];
        
        fprintf(file, "#define %.*s(n) %.*s n%.*s\n",
                macro_string.size, macro_string.str,
                ret_string.size, ret_string.str,
                args_string.size, args_string.str
                );
    }
    
    fprintf(file, "extern \"C\"{\n");
    for (int i = 0; i < sig_count; ++i){
        String name_string  = function_set.name[i];
        String macro_string = function_set.macros[i];
        
        fprintf(file, "    typedef %.*s(%.*s_Function);\n",
                macro_string.size, macro_string.str,
                name_string.size, name_string.str);
    }
    fprintf(file, "}\n");
    
    fprintf(file, "struct Application_Links{\n");
    fprintf(file,
            "    void *memory;\n"
            "    int memory_size;\n"
            );
    for (int i = 0; i < sig_count; ++i){
        String name_string  = function_set.name[i];
        String public_string = function_set.public_name[i];
        
        fprintf(file, "    %.*s_Function *%.*s;\n",
                name_string.size, name_string.str,
                public_string.size, public_string.str);
    }
    fprintf(file,
            "    void *cmd_context;\n"
            "    void *system_links;\n"
            "    void *current_coroutine;\n"
            "    int type_coroutine;\n"
            );
    fprintf(file, "};\n");
    
    fprintf(file, "#define FillAppLinksAPI(app_links) do{");
    for (int i = 0; i < sig_count; ++i){
        String name = function_set.name[i];
        String public_string = function_set.public_name[i];
        
        fprintf(file,
                "\\\n"
                "app_links->%.*s = %.*s;",
                public_string.size, public_string.str,
                name.size, name.str
                );
    }
    fprintf(file, " } while(false)\n");
    
    fclose(file);
    
    // NOTE(allen): Documentation
    file = fopen(API_DOC, "wb");
    
#define CODE_STYLE "font-family: \"Courier New\", Courier, monospace; text-align: left;"
    
#define BACK_COLOR "#FFFFE0"
#define POP_COLOR_1 "#007000"
#define POP_BACK_1 "#E0FFD0"
#define POP_COLOR_2 "#007070"
#define POP_COLOR_3 "#005000"
    
    fprintf(file,
            "<html lang=\"en-US\">\n"
            "<head>\n"
            "<title>4coder API Docs</title>\n"
            "<style>\n"
            
            "body { "
            "background: " BACK_COLOR "; "
            "}\n"
            
            // H things
            "h1,h2,h3,h4 { "
            "color: " POP_COLOR_1 "; "
            "margin: 0; "
            "}\n"
            
            // ANCHORS
            "a { "
            "color: " POP_COLOR_1 "; "
            "text-decoration: none; "
            "}\n"
            "a:visited { "
            "color: " POP_COLOR_2 "; "
            "}\n"
            "a:hover { "
            "background: " POP_BACK_1 "; "
            "}\n"
            
            // LIST
            "ul { "
            "list-style: none; "
            "padding: 0; "
            "margin: 0; "
            "}\n"
            "li { "
            "padding-left: 1em;"
            "text-indent: -.7em;"
            "}\n"
            "li:before { "
            "content: \"4\"; "
            "color: " POP_COLOR_3 "; "
            "font-family:\"Webdings\"; "
            "}\n"
            
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<div style='"
            "font-family:Arial; "
            "position: absolute; "
            "left: 10mm; "
            "width: 180mm; "
            "text-align: justify; "
            "line-height: 1.25;'>\n"
            "<h1 style='margin-top: 5mm; margin-bottom: 5mm;'>4coder API</h1>\n"
            );
    
    fprintf(file,
            "<h2>&sect;1 Introduction</h2>\n"
            "<div>\n"
            
            "<p>\n"
            "This is the documentation for " VERSION " The documentation has been made as "
            "accurate as possible but there may be errors. If you have questions or "
            "discover errors please contact <span style='"CODE_STYLE"'>editor@4coder.net</span>."
            "</p>\n"
            
            "<p>\n"
            "</p>\n"
            
            "</div>\n");
    
    fprintf(file, "<h2>&sect;2 Types and Functions</h2>\n");
    {
#undef SECTION
#define SECTION "2.1"
        
        fprintf(file,
                "<h3>&sect;"SECTION" Function List</h3>\n"
                "<ul>\n");
        
        for (int i = 0; i < sig_count; ++i){
            String name = function_set.public_name[i];
            fprintf(file,
                    "<li>\n"
                    "<a href='#%.*s_doc'>%.*s</a>\n"
                    "</li>\n",
                    name.size, name.str,
                    name.size, name.str
                    );
        }
        fprintf(file, "</ul>\n");
        
#undef SECTION
#define SECTION "2.2"
        
        fprintf(file, "<h3 style='margin-top: 5mm; margin-bottom: 5mm;'>&sect;"SECTION" Descriptions</h3>\n");
        for (int i = 0; i < sig_count; ++i){
            String name = function_set.public_name[i];
            String ret = function_set.ret[i];
            String args = function_set.args[i];
            String doc = function_set.doc_string[i];
            
            if (doc.size <= 0){
                doc = make_lit_string("No doc generated ~ assume this call is not meant to be public");
                printf("warning: missing info for %.*s\n", name.size, name.str);
            }
            
            fprintf(file,
                    "<div id='%.*s' style='margin-bottom: 1cm;'>\n"
                    " <h4>&sect;"SECTION".%d: %.*s</h4>\n"
                    " <div style='"CODE_STYLE" margin-top: 3mm; margin-bottom: 3mm; font-size: .95em;'>",
                    name.size, name.str, i,
                    name.size, name.str
                    );
            
            fprintf(file, "%.*s %.*s", ret.size, ret.str, name.size, name.str);
            // TODO(allen): replace this with a loop to write each parameter on it's own line
            fprintf(file, "%.*s", args.size, args.str);
            
            fprintf(file, 
                    "</div>\n"
                    " <div>%.*s</div>\n"
                    "</div>\n",
                    doc.size, doc.str
                    );
        }
    }
    
    fprintf(file,
            "</div>\n"
            "</body>\n"
            "</html>\n"
            );
    
    fclose(file);

    return(filename);
}

int main(){
    char *filename = 0;
        
    filename = generate_keycode_enum();
    printf("gen success: %s\n", filename);
        
    filename = generate_style();
    printf("gen success: %s\n", filename);
        
    filename = generate_custom_headers();
    printf("gen success: %s\n", filename);
}

// BOTTOM

