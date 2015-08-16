
#ifndef __paiv_hexagons_platform_hpp
#define __paiv_hexagons_platform_hpp

namespace paiv {
  
struct Layout {
    const Orientation orientation;
    const Point size;
    const Point origin;
    Layout(Orientation orientation_, Point size_, Point origin_)
    : orientation(orientation_), size(size_), origin(origin_) {}
};

Point hex_to_pixel(Layout layout, Hex h) {
    const Orientation& M = layout.orientation;
    double x = (M.f0 * h.q + M.f1 * h.r) * layout.size.x;
    double y = (M.f2 * h.q + M.f3 * h.r) * layout.size.y;
    return {x + layout.origin.x, y + layout.origin.y};
}

FractionalHex pixel_to_hex(Layout layout, Point p) {
    const Orientation& M = layout.orientation;
    Point pt = {(p.x - layout.origin.x) / layout.size.x,
        (p.y - layout.origin.y) / layout.size.y};
    double q = M.b0 * pt.x + M.b1 * pt.y;
    double r = M.b2 * pt.x + M.b3 * pt.y;
    return {q, r};
}

Point hex_corner_offset(Layout layout, int corner) {
    Point size = layout.size;
    double angle = 2.0 * M_PI *
             (corner + layout.orientation.start_angle) / 6;
    return {size.x * cos(angle), size.y * sin(angle)};
}

vector<Point> polygon_corners(Layout layout, Hex h) {
    vector<Point> corners = {};
    Point center = hex_to_pixel(layout, h);
    for (int i = 0; i < 6; i++) {
        Point offset = hex_corner_offset(layout, i);
        corners.push_back({center.x + offset.x, center.y + offset.y});
    }
    return corners;
}

}

#endif
