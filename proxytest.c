#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>


enum {
	WINDOW_WIDTH = 640,
	WINDOW_HEIGHT = 480,
};


int
main(int argc, char **argv){
	Ecore_Evas *ee;
	Evas *e;
	Evas_Object *bg, *img, *proxy;
	bool rv;
	int w,h;

	ecore_init();
	ecore_evas_init();

	/* FIXME: Also support GL engine */
	ee = ecore_evas_software_x11_new(NULL,0,0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
	if (!ee){
		printf("Unable to create ecore evas\n");
		return 1;
	}

	e = ecore_evas_get(ee);
	if (!e){
		printf("Unable to get 'e'\n");
		return 1;
	}

	bg = evas_object_rectangle_add(e);
	evas_object_color_set(bg, 200,200,200,255);
	/* FIXME: Scale to window size */
	evas_object_resize(bg, WINDOW_WIDTH, WINDOW_HEIGHT);
	evas_object_show(bg);

	img = evas_object_image_filled_add(e);
	evas_object_image_file_set(img, "lucasstamp.jpg", NULL);
	evas_object_image_size_get(img, &w, &h);
	evas_object_resize(img, w, h);
	evas_object_move(img, 10,10);
	evas_object_show(img);

	proxy = evas_object_proxy_add(e);
	if (!proxy){
		printf("Unable to create proxy object\n");
		return 1;
	}
	rv = evas_object_proxy_source_set(proxy, img);
	if (rv != true){
		printf("Error setting proxy source\n");
		return 1;
	}
	evas_object_resize(proxy, w, h);
	evas_object_move(proxy, 20 + w, 10);
	evas_object_show(proxy);

	ecore_evas_show(ee);
	ecore_main_loop_begin();

	return 0;
}


