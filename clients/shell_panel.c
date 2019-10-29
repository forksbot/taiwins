#include "shell.h"


/*******************************************************************************
 * widgets
 ******************************************************************************/

static void
widget_should_close(void *data, struct tw_ui *ui_elem)
{
	struct widget_launch_info *info = (struct widget_launch_info *)data;
	struct shell_widget *widget = info->current;

	tw_ui_destroy(widget->proxy);
	app_surface_release(&widget->widget);
	widget->proxy = NULL;
	info->current = NULL;
}

static struct  tw_ui_listener widget_impl = {
	.close = widget_should_close,
};

//later we can take advantage of the idle queue for this.
void
launch_widget(struct app_surface *panel_surf)
{
	struct shell_output *shell_output =
		container_of(panel_surf, struct shell_output, panel);
	struct desktop_shell *shell = shell_output->shell;
	struct widget_launch_info *info = &shell->widget_launch;
	if (info->current == info->widget)
		return;
	else if (info->current != NULL) {
		//if there is a widget launched and is not current widget
		app_surface_release(&info->current->widget);
		info->current = NULL;
	}

	struct wl_surface *widget_surface =
		wl_compositor_create_surface(shell->globals.compositor);
	struct tw_ui *widget_proxy =
		tw_shell_launch_widget(shell->interface, widget_surface,
				       shell_output->index, info->x, info->y);

	info->widget->proxy = widget_proxy;
	tw_ui_add_listener(widget_proxy, &widget_impl, info);
	//launch widget
	app_surface_init(&info->widget->widget, widget_surface,
			 panel_surf->wl_globals,
			 APP_SURFACE_WIDGET, APP_SURFACE_NORESIZABLE);
	nk_cairo_impl_app_surface(&info->widget->widget, shell->widget_backend,
				  info->widget->draw_cb,
				  make_bbox(info->x, info->y,
					    info->widget->w, info->widget->h,
					    shell_output->bbox.s));

	app_surface_frame(&info->widget->widget, false);

	info->current = info->widget;
}

/*******************************************************************************
 * shell panel
 ******************************************************************************/
static inline struct nk_vec2
widget_launch_point_flat(struct nk_vec2 *label_span, struct shell_widget *clicked,
			 struct app_surface *panel_surf)
{
	struct shell_output *shell_output =
		container_of(panel_surf, struct shell_output, panel);
	int w = panel_surf->allocation.w;
	int h = panel_surf->allocation.h;
	struct nk_vec2 info;
	if (label_span->x + clicked->w > w)
		info.x = w - clicked->w;
	else if (label_span->y - clicked->w < 0)
		info.x = label_span->x;
	else
		info.x = label_span->x;
	//this totally depends on where the panel is
	if (shell_output->shell->panel_pos == TW_SHELL_PANEL_POS_TOP)
		info.y = h;
	else {
		info.y = shell_output->bbox.h -
			panel_surf->allocation.h -
			clicked->h;
	}
	return info;
}

static void
shell_panel_measure_leading(struct nk_context *ctx, float width, float height,
			    struct app_surface *panel_surf)
{
	struct shell_output *shell_output =
		container_of(panel_surf, struct shell_output, panel);
	struct desktop_shell *shell = shell_output->shell;
	nk_text_width_f text_width = ctx->style.font->width;
	struct shell_widget_label widget_label;
	struct shell_widget *widget = NULL;

	double total_width = 0.0;
	int h = panel_surf->allocation.h;
	size_t n_widgets =  wl_list_length(&shell->shell_widgets);
	nk_layout_row_begin(ctx, NK_STATIC, h - 12, n_widgets);
	wl_list_for_each(widget, &shell->shell_widgets, link) {
		int len = widget->ancre_cb(widget, &widget_label);
		double width =
			text_width(ctx->style.font->userdata,
				   ctx->style.font->height,
				   widget_label.label, len);
		nk_layout_row_push(ctx, width+10);
		/* struct nk_rect bound = nk_widget_bounds(ctx); */
		nk_button_text(ctx, widget_label.label, len);
		total_width += width+10;
	}
	shell_output->widgets_span = total_width;
}

static void
shell_panel_frame(struct nk_context *ctx, float width, float height,
		  struct app_surface *panel_surf)
{
	struct shell_output *shell_output =
		container_of(panel_surf, struct shell_output, panel);
	struct desktop_shell *shell = shell_output->shell;
	struct shell_widget_label widget_label;
	nk_text_width_f text_width = ctx->style.font->width;
	//drawing labels
	size_t n_widgets =  wl_list_length(&shell->shell_widgets);
	struct shell_widget *widget = NULL, *clicked = NULL;
	struct nk_vec2 label_span = nk_vec2(0, 0);

	int h = panel_surf->allocation.h;
	int w = panel_surf->allocation.w;

	nk_layout_row_begin(ctx, NK_STATIC, h-12, n_widgets+1);
	int leading = w - (int)(shell_output->widgets_span+0.5)-20;
	nk_layout_row_push(ctx, leading);
	nk_spacing(ctx, 1);

	wl_list_for_each(widget, &shell->shell_widgets, link) {
		int len = widget->ancre_cb(widget, &widget_label);
		double width =
			text_width(ctx->style.font->userdata,
				   ctx->style.font->height,
				   widget_label.label, len);

		nk_layout_row_push(ctx, width+10);
		struct nk_rect bound = nk_widget_bounds(ctx);
		if (nk_widget_is_mouse_clicked(ctx, NK_BUTTON_LEFT)) {
			clicked = widget;
			label_span.x = bound.x;
			label_span.y = bound.x+bound.w;
		}
		nk_button_text_styled(ctx, &shell_output->shell->label_style,
				      widget_label.label, len);
	}
	nk_layout_row_end(ctx);
	//widget launch condition, clicked->proxy means widget already clicked
	if (!clicked || clicked->proxy ||
	    !clicked->draw_cb ||
	    shell->transient.wl_surface) //if other surface is ocuppying
		return;
	struct widget_launch_info *info = &shell->widget_launch;
	info->widget = clicked;
	struct nk_vec2 p = widget_launch_point_flat(&label_span, clicked, panel_surf);
	info->x = (int)p.x;
	info->y = (int)p.y;
	nk_wl_add_idle(ctx, launch_widget);
}



void
shell_init_panel_for_output(struct shell_output *w)
{
	struct desktop_shell *shell = w->shell;
	struct wl_surface *pn_sf;

	//at this point, we are  sure to create the resource
	pn_sf = wl_compositor_create_surface(shell->globals.compositor);
	w->pn_ui = tw_shell_create_panel(shell->interface, pn_sf, w->index);
	app_surface_init(&w->panel, pn_sf, &shell->globals,
			 APP_SURFACE_PANEL, APP_SURFACE_NORESIZABLE);
	nk_cairo_impl_app_surface(&w->panel, shell->panel_backend,
				  shell_panel_frame,
				  make_bbox_origin(w->bbox.w, shell->panel_height,
						   w->bbox.s));

	struct shell_widget *widget;
	wl_list_for_each(widget, &shell->shell_widgets, link) {
		shell_widget_hook_panel(widget, &w->panel);
		shell_widget_activate(widget, &shell->globals.event_queue);
	}

	nk_wl_test_draw(shell->panel_backend, &w->panel,
			shell_panel_measure_leading);
}

void
shell_resize_panel_for_output(struct shell_output *w)
{
	nk_wl_test_draw(w->shell->panel_backend, &w->panel,
			shell_panel_measure_leading);

	w->panel.flags &= ~APP_SURFACE_NORESIZABLE;
	app_surface_resize(&w->panel, w->bbox.w, w->shell->panel_height,
			   w->bbox.s);
	w->panel.flags |= APP_SURFACE_NORESIZABLE;
}