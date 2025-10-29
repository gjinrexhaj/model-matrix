//
// Created by Gjin on 10/29/25.
//

#pragma once
#include <vector>

// TODO: test grid class func in test harness
// TODO: impl debug logging method for 3d grid
// TODO: impl dynamic resizing

class Grid
{
    public:
        // Constructor
        Grid(int w, int h, int l) :
            width(w), height(h), length(l)
            {
                data.resize(width * height * length);
            };

        int read(int xIndex, int yIndex, int zIndex)
        {
            return getIndex(xIndex, yIndex, zIndex);
        }

        void write(int xIndex, int yIndex, int zIndex, int state)
        {
            data[getIndex(xIndex, yIndex, zIndex)] = state;
        }


    private:
        int width, height, length;
        std::vector<int> data;

        // Retrieve data of index
        int getIndex(int x, int y, int z) const
        {
            if (x >= width || y >= height || z >= length) {
                throw std::out_of_range("Grid3D: Index out of bounds");
            }
            return z * (width * height) + y * width + x;
        }
};
