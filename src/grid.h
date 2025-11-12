//
// Created by Gjin on 10/29/25.
//

#pragma once
#include <vector>

// TODO: impl dynamic resizing

class Grid
{
    public:
        // Constructor
        Grid(int w, int h, int d) :
            width(w), height(h), depth(d)
            {
                data.resize(width * height * depth);
            };

        int read(int xIndex, int yIndex, int zIndex)
        {
            return data[getIndex(xIndex, yIndex, zIndex)];
        }

        void write(int xIndex, int yIndex, int zIndex, int state)
        {
            data[getIndex(xIndex, yIndex, zIndex)] = state;
        }

        unsigned int getWidth() const { return width; }
        unsigned int getHeight() const { return height; }
        unsigned int getDepth() const { return depth; }


    private:
        int width, height, depth;
        std::vector<int> data;

        // Retrieve data of index
        int getIndex(int x, int y, int z) const
        {
            if (x >= width || y >= height || z >= depth) {
                throw std::out_of_range("Grid3D: Index out of bounds");
            }
            return z * (width * height) + y * width + x;
        }
};
