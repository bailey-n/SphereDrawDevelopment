//
// Created by Nathan on 1/29/2026.
//

#ifndef SPHEREDRAW_DRAWABLE_H
#define SPHEREDRAW_DRAWABLE_H

#include "camera.h"

class Drawable {
    enum Form {
        None = 0,
        Point = 1,
        Polyline = 2,
        PolygonBoundary = 3,
        Polygon = 4,
    };

    Form form;
    virtual void draw(const Camera& camera) = 0;
};


#endif //SPHEREDRAW_DRAWABLE_H
