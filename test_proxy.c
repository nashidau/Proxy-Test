//gcc -g `pkg-config enlightenment --libs --cflags` test_proxy.c 
#include <stdbool.h>

#include <Eina.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore.h>
#include <Ecore_Evas.h>


static Evas_Object *obj, *o, *bg;

static Eina_Bool
_timer(void *data)
{
   if (evas_object_visible_get(obj))
     evas_object_hide(obj);
   else
     evas_object_show(obj);
   
   return true;
}

int main(int argc, char *argv[])
{
   Ecore_Evas *ee;
   Evas *e;
   int w, h;

   w = h = 180;

   ecore_init();
   ecore_evas_init();
   edje_init();
   
   ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 200, 200);
   if (!ee)
	   ee = ecore_evas_software_x11_new(NULL, 0, 10, 10, 200, 200);
   ecore_evas_alpha_set(ee, 1);
   e = ecore_evas_get(ee);
   
   bg = evas_object_rectangle_add(e);
   evas_object_resize(bg, 200, 200);
   evas_object_color_set(bg, 100, 100, 100, 255); 
   evas_object_show(bg);

   ecore_evas_name_class_set(ee, "BOX", "TEST"); 
   ecore_evas_show(ee);

   /* obj = evas_object_image_filled_add(e);
    * evas_object_image_file_set(obj, "/home/jeff/.icons/Faenza/actions/scalable/fileopen.svg", NULL);
    * evas_object_image_preload(obj, 0);
    * evas_object_image_load_size_set(obj, w/2, h/2); */

   obj = edje_object_add(e); 
   int ok = edje_object_file_set(obj, "e-module-scale.edj", "icon");
   printf("loaded %d %d\n", ok, edje_object_load_error_get(obj));

   evas_object_resize(obj, w/4, h/4);   
   evas_object_move(obj, w/2, h/2);
   evas_object_show(obj);

   o = evas_object_image_add(e); 
   evas_object_image_source_set(o, obj); 
   evas_object_image_fill_set(o, 0,0, w/2, h/2);   
   evas_object_resize(o, w/2, h/2);
   evas_object_show(o);
   
   ecore_timer_add(0.1, _timer, NULL);
   
   ecore_main_loop_begin();
   
   evas_object_del(o); 
   evas_object_del(obj);
   evas_object_del(bg); 

   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   return 0;
}
