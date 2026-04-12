#ifndef SVGee_H
#define SVGee_H

#include <SDL.h>
#include "ok_lib.h"
#include "basics.h"
#include "geometry.h"

#define BEZ_REZ_QUO 0.125f


typedef struct {
    int tag_index;      // Index into SVG_Layer's tags array
    char *data;
} Metadata;

typedef struct svg_element_struct SVG_Element;

typedef struct svg_element_struct{
    char id [64];
    enum {SVG_NULL, SVG_GEO, SVG_GROUP, SVG_OTHER} type;
    union{
        Geometric geo;
        SVG_Element *group; // vec
        char *other;        // unknown <tag>, just a string containing the tag's label.
    } u;
    Style *style;       // reference into layer's styles array
    Metadata* metadata; // vec
} SVG_Element;


typedef struct {
    SVG_Element *E;      // vec
    Metadata* metadata;  // vec
    char** tags;         // Unique tag names (vec)
    Style **styles;      // vec
} SVG_Layer;


int get_else_push_style( Style *style, map_int_int *style_map, Style ***style_vec );

SVG_Layer* svg_load_layer( SDL_IOStream* f, const char* layer_label );

Styled_Geo SVG_Element_to_Styled_Geo( SVG_Element *E );
void SVG_Layer_to_Styled_Geo_vec( SVG_Layer *L, Styled_Geo **SG );

Geo_Animation SVG_Element_to_Geo_Animation( SVG_Layer *L, SVG_Element *E );

void SVG_Layer_destroy( SVG_Layer *layer );
void SVG_Layer_dump(const SVG_Layer* layer);

#endif