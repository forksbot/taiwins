# Weston-desktop API

The API essentially implements the `xdg_shell_protocols`, it combines the
`wl_shell`,`xdg_shell_v5` and `xdg_shell_v6` under one implementation, which is
really smart, saves much resources for the developers.

## weston_layer
`weston_layer`s are stored from top to bottom in the compositor, in that case
`compositor->layer_header->top_layer->...bottom_layer`. The same priority
pattern applies within the layer. So the top view is the first view in the
layer's `view_link`. When compositor starts painting, it will build the
view_list from all the layers(priority top to down). With the information, the
compositor can figure out which parts of the `weston_plane` (a `pixman_region`)
are damaged.

## weston_plane
The `weston_plane` is a logical representation of the hardware compositing
planes, so we only need to do the compositing on every plane, and the hardware
composits the different planes. Currently here is a piece of code inside the
`gl-renderer` and `pixman-renderer`.

	wl_list_for_each_reverse(view, compositor->view_list, link)
		if (view->plane == &compositor->primary_plane)
			draw_view(view);

## the pixman_region_t
`pixman_region16_t` or `pixman_region32_t` are both regions, because it contains
boxes(x,y,w,h). You have to walk though the pixman examples to know how to use
it.

## wl_frame
The key function is `weston_output_repaint`. You will see the accumulation of
`frame_callback`, `weston_compositor_build_view_list`, and call to
`renderer->repaint`, then send done to all the `frame_callback`.

## Taiwins-desktop implementation
desktop has following concepts:
	- **workspace**  one workspace contains all the views from different **output**.
	- **layer** tiling layer, floating layer, hidden layer.
	- **layout** can work on per **output** space or directly under
	  compositor. The (tiling) layout is internally tree like structure.
  layers **CARE** about the order, layouts **cares** about the positions and
	  sizes, so they should not touch each others responsibilities.

### sample process
	- adding a window in the workspace
		* decide to put in the floating layer or tiling layer.
		* you need to have the output from `tw_get_focused_output`.
		* find the **layout** corresponded to the **output**.
		* decide how you want to tell the **layout** new view is ready.
		* `disposer` all the views in the layout or just the one.
	- deleting a window.
		* decide if it is in the floating/tiling layer
		* decide how to announce the deleting of the view.
		* `disposer` all the views or not.
	- focus the window.
		* throw it to the top of the view_list.
	- moving up or down from the current view.
		* modify the positin of the view in the tree?

### peusdo code

	//at event point.
	desktop.make_event(event_type, view) >> layout;
	layout.prod_layout() >> desktop;

### disposing algorithm
	It is too early to talk about this, right now the priority is implementing a
	stupid algorithm.

### How do we use it
The `desktop`, `shell`, or any other code we write, are meant to provide the
functionalities and there are only a few ways to expose them to users. The
desktop need to provide the means to operate the `views`, like moving `views`,
delete them, switching workspaces and so on. They are in the form of bindings.

The second way to provide the functionalities is through protocols. They are
mainly used in `shell`.

We can implement them by keybindings, they are mutual exclusive, if both are
activated. The effects doubled. Keybinding has another problem,
customization. For pointer and touch, you can easily set moving up and done as


## weston-renderer implementation
The one key function to look at is the `weston_output_repaint`. This is the
frame callback, called in maybe `eventloop`. It does a few things:

- it need to build up the `view_list` of current frame, extract from layers, and
  sort them in order.

- move views to planes, if backend supports multiple planes, it does that.

- initialize the `frame_callback` list, this is where the `frame_done` gets sent

- next it accumenate the damage, when you set `weston_view_damage_ below`, it
  gets accumulated here.

- Then it calls renderer to repaint.

- sends all the `frame_callbacks`.

- output has an animation list. It does that as well.

## weston-view coordinates
see the `compositor_accumulate_damamge`, there is nothing about output
weston_view has a geometry, this geometry is in global coordinates.
