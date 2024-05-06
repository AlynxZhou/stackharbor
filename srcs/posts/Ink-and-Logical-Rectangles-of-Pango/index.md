---
title: Pango 中的 Ink 和 Logical 矩形
layout: post
#comment: true
created: 2024-05-06T17:08:22
categories:
  - 计算机
  - 编程
tags:
  - 计算机
  - 编程
  - 字体
---
暂时没有时间完成这篇博客，但为了能在注释里引用这个链接所以先放个占位符在这里。

<!--more-->

总之下面是示例程序：

```c
#include <gtk/gtk.h>

static void draw(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data)
{
	PangoLayout *layout = pango_cairo_create_layout(cr);
	pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_NONE);
	PangoFontDescription *font = pango_font_description_new();
	pango_font_description_set_family(font, "serif");
	pango_font_description_set_style(font, PANGO_STYLE_ITALIC);
	pango_font_description_set_absolute_size(font, 200 * PANGO_SCALE);
	pango_layout_set_font_description(layout, font);

	PangoRectangle string_ink;
	PangoRectangle string_logical;
	pango_layout_set_text(layout, "g", -1);
	pango_layout_get_pixel_extents(layout, &string_ink, &string_logical);
	g_print("Logical: x is %d, y is %d, width is %d, height is %d.\n", string_logical.x, string_logical.y, string_logical.width, string_logical.height);
	g_print("Ink: x is %d, y is %d, width is %d, height is %d.\n", string_ink.x, string_ink.y, string_ink.width, string_ink.height);

	cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
	cairo_paint(cr);

	const int x = 100;
	const int y = 100;
	cairo_move_to(cr, x, y);
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
	pango_cairo_show_layout(cr, layout);

	cairo_set_line_width(cr, 5);
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
	cairo_rectangle(cr, x + string_logical.x, y + string_logical.y, string_logical.width, string_logical.height);
	cairo_stroke(cr);

	cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
	cairo_rectangle(cr, x + string_ink.x, y + string_ink.y, string_ink.width, string_ink.height);
	cairo_stroke(cr);
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
	GtkWidget *window = gtk_application_window_new (app);
	gtk_window_set_title(GTK_WINDOW(window), "Test Pango Extents");
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
	GtkWidget *area = gtk_drawing_area_new();
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(area), draw, NULL, NULL);
	gtk_window_set_child(GTK_WINDOW(window), area);
	gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
	GtkApplication *app = gtk_application_new("one.alynx.test-pango-extents", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
```

总之下面是编译命令：

```shell
$ gcc -o test-pango-extents `pkg-config --cflags --libs gtk4` test-pango-extents.c
$ ./test-pango-extents
```

总之下面是运行截图：

![test-pango-extents](./screenshot.png)
