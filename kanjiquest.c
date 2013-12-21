/*
 * Copyright (c) 2013 Martin Rödel aka Yomin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <cairo/cairo.h>

#define NAME "Kanji Quest"
#define TITLE "漢字探求"
#define HEIGHT 200
#define WIDTH 500
#define FONT "TakaoMincho"
#define FONTSIZE 60
#define BUFSIZE 40

/* vocab file format
 * %i ignore
 * %h hiragana/katakana
 * %k kanji
 * %r heisig
 * %e english
 * %ax alternative delimiter x
 *
 * default format examples
 * shougo^しょうご^正午^correct+noon|noon
 * kawaii^かわいい^可愛い^can+love+い|cute/pretty
 * honya/shoten^ほんや/しょてん^本屋/書店^book+roof/write+store|bookstore
 */
#define FORMAT "%a/%i^%h^%k^%r|%e/"

struct vocab
{
    char hira[BUFSIZE];
    char kanji[BUFSIZE];
    char heisig[BUFSIZE];
    char en[BUFSIZE];
    char alt[3];
};

struct quest
{
    struct vocab v;
    char *q, *font;
    int height, width, fsize;
};

gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    struct quest *q = (struct quest*) data;
    cairo_text_extents_t te;
    int fsize;
    
    cairo_rectangle(cr, 0, 0, q->width, q->height);
    cairo_fill(cr);
    
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_select_font_face(cr, q->font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    
    cairo_set_font_size(cr, q->fsize-20);
    cairo_text_extents(cr, TITLE, &te);
    cairo_move_to(cr, q->width/2-te.x_bearing-te.width/2, q->height/3);
    cairo_show_text(cr, TITLE);
    
    cairo_set_font_size(cr, q->fsize);
    cairo_text_extents(cr, q->q, &te);
    fsize = q->fsize;
    while(te.width >= q->width)
    {
        cairo_set_font_size(cr, --fsize);
        cairo_text_extents(cr, q->q, &te);
    }
    
    cairo_move_to(cr, q->width/2-te.x_bearing-te.width/2, q->height/2+te.height);
    cairo_show_text(cr, q->q);
    
    return TRUE;
}

gboolean keypress(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    struct quest *q = (struct quest*) data;
    
    switch(event->keyval)
    {
    case GDK_KEY_1: q->q = q->v.hira; break;
    case GDK_KEY_2: q->q = q->v.kanji; break;
    case GDK_KEY_3: q->q = q->v.heisig; break;
    case GDK_KEY_4: q->q = q->v.en; break;
    case GDK_KEY_q:
    case GDK_KEY_space:
    case GDK_KEY_Return: gtk_main_quit(); break;
    default: return FALSE;
    }
    
    gtk_widget_queue_draw(widget);
    
    return TRUE;
}

inline void match(char **line, char **format)
{
    while(**line && **format && **line == **format && **format != '%')
    {
        (*line)++;
        (*format)++;
    }
}

inline void forward(char **line, char delim)
{
    while(**line && **line != delim)
        (*line)++;
}

inline void readin(char **line, char delim, char *dst)
{
    int n = 0;
    
    while(**line && **line != delim)
    {
        if(n < BUFSIZE-1)
        {
            *dst = **line == '\n' ? 0 : **line;
            dst++;
            n++;
        }
        (*line)++;
    }
}

int parse(char *line, char *format, struct vocab *v)
{
    memset(v, 0, sizeof(struct vocab));
    char alt = 0;
    
    while(1)
    {
        match(&line, &format);
        if(!*line || !*format || *format != '%')
            break;
        switch(format[1])
        {
        case 'i':
            forward(&line, format[2]);
            break;
        case 'h':
            readin(&line, format[2], v->hira);
            v->alt[0] = alt;
            break;
        case 'k':
            readin(&line, format[2], v->kanji);
            v->alt[1] = alt;
            break;
        case 'r':
            readin(&line, format[2], v->heisig);
            v->alt[2] = alt;
            break;
        case 'e':
            readin(&line, format[2], v->en);
            break;
        case 'a':
            alt = format[2];
            break;
        default:
            return 1;
        }
        format += 3;
        if(!*line || !*format)
            break;
        line++;
    }
    
    if(*v->hira && *v->kanji && *v->heisig && *v->en)
        return 0;
    
    return 1;
}

inline int count_alt(char *str, char alt)
{
    int n = 1;
    while(*str && (str = strchr(str, alt)))
    {
        n++;
        str++;
    }
    return n;
}

inline void copy_alt(char *dst, char *src, char alt, int pos)
{
    int n = 0;
    char *ptr = src-1;
    
    if(alt)
    {
        while(n++ <= pos)
        {
            src = ptr+1;
            ptr = strchr(src, alt);
        }
        if(ptr)
            *ptr = 0;
    }
    strcpy(dst, src);
}

void usage(char *name)
{
    printf("Usage: %s [-v <format>] [-f <font>] [-s <fontsize>] [-h <height>] [-w <width>] <vocabfile> [<vocabfile>[...]]\n", name);
    exit(1);
}

int main(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *area;
    
    FILE *f;
    
    char *line = 0;
    size_t size = 0;
    
    struct vocab *vocab;
    int vocabs, v, alt;
    
    struct quest q;
    
    int opt;
    char *format = FORMAT, *ptr;
    
    q.font = FONT;
    q.fsize = FONTSIZE;
    q.height = HEIGHT;
    q.width = WIDTH;
    
    while((opt = getopt(argc, argv, "v:f:s:h:w:")) != -1)
    {
        switch(opt)
        {
        case 'v':
            format = optarg;
            break;
        case 'f':
            q.font = optarg;
            break;
        case 's':
            if(strspn(optarg, "1234567890") != strlen(optarg))
                usage(argv[0]);
            q.fsize = atoi(optarg);
            break;
        case 'h':
            if(strspn(optarg, "1234567890") != strlen(optarg))
                usage(argv[0]);
            q.height = atoi(optarg);
            break;
        case 'w':
            if(strspn(optarg, "1234567890") != strlen(optarg))
                usage(argv[0]);
            q.width = atoi(optarg);
            break;
        default:
            usage(argv[0]);
        }
    }
    
    ptr = format;
    while(*ptr)
    {
        if(*ptr == '%')
        {
            ptr++;
            if(!*ptr || (*ptr == 'a' && (!ptr[1] || ptr[1] == '%')))
            {
                printf("Format malformed\n");
                return 2;
            }
            if(*ptr == 'a')
                ptr++;
        }
        ptr++;
    }
    
    if(optind == argc)
        usage(argv[0]);
    
    vocabs = 20;
    vocab = malloc(vocabs*sizeof(struct vocab));
    v = 0;
    
    while(optind < argc)
    {
        if(!(f = fopen(argv[optind], "r")))
        {
            perror("Failed to open vocab file");
            free(vocab);
            return 3;
        }
        
        while(getline(&line, &size, f) != -1)
        {
            if(parse(line, format, &vocab[v]))
                continue;
            v++;
            if(v == vocabs)
            {
                vocabs += 20;
                vocab = realloc(vocab, vocabs*sizeof(struct vocab));
            }
        }
        
        if(ferror(f))
        {
            printf("Failed to read vocab file\n");
            free(vocab);
            return 4;
        }
        
        fclose(f);
        optind++;
    }
    
    if(line)
        free(line);
    
    if(!v)
    {
        printf("No vocab matches format\n");
        free(vocab);
        return 5;
    }
    
    srand(time(0));
    v = rand() % v;
    alt = rand() % count_alt(vocab[v].kanji, vocab[v].alt[1]);
    copy_alt(q.v.hira, vocab[v].hira, vocab[v].alt[0], alt);
    copy_alt(q.v.kanji, vocab[v].kanji, vocab[v].alt[1], alt);
    copy_alt(q.v.heisig, vocab[v].heisig, vocab[v].alt[2], alt);
    strcpy(q.v.en, vocab[v].en);
    free(vocab);
    q.q = q.v.kanji;
    
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), NAME);
    gtk_window_set_default_size(GTK_WINDOW(window), q.width, q.height);
    
    area = gtk_drawing_area_new();
    gtk_widget_set_size_request(area, q.width, q.height);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "draw", G_CALLBACK(draw), &q);
    g_signal_connect(window, "key-press-event", G_CALLBACK(keypress), &q);
    
    gtk_container_add(GTK_CONTAINER(window), area);
    
    gtk_widget_show(area);
    gtk_widget_show(window);
    
    gtk_main();
    
    return 0;
}
