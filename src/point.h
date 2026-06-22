#ifndef POINT_H
#define POINT_H

class Point {
    private:
        float m_x;
        float m_y;

    public:
        Point();
        Point(float x, float y);

        float x() const;
        float y() const;
};

#endif // POINT_H