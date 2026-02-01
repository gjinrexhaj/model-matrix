//
// Created by Gjin on 10/29/25.
//

#pragma once
#include <stdexcept>
#include <vector>

class Grid
{
    public:
        // Constructor
        Grid(const int w, const int h, const int d) :
            width(w), height(h), depth(d)
            {
                data.resize(width * height * depth);
            };

        int read(const int xIndex, const int yIndex, const int zIndex)
        {
            return data[getIndex(xIndex, yIndex, zIndex)];
        }

        void write(const int xIndex, const int yIndex, const int zIndex, const int state)
        {
            data[getIndex(xIndex, yIndex, zIndex)] = state;
        }

        void resize(const int newW, const int newH, const int newD)
        {
            // create new data buffer
            std::vector<int> newData(newW * newH * newD);

            const int cpyWidth = std::min(width,  newW);
            const int cpyHeight = std::min(height, newH);
            const int cpyDepth = std::min(depth,  newD);

            // copy overlapping region
            for (int z = 0; z < cpyDepth; ++z)
            {
                for (int y = 0; y < cpyHeight; ++y)
                {
                    for (int x = 0; x < cpyWidth; ++x)
                    {
                        int oldIndex = z * (width * height) + y * width + x;
                        int newIndex = z * (newW * newH) + y * newW + x;
                        newData[newIndex] = data[oldIndex];
                    }
                }
            }

            // apply resize buffer to new grid, change internal state accordingly
            data = std::move(newData);
            width  = newW;
            height = newH;
            depth  = newD;
        }


        std::vector<int> getDataVector()
        {
            return data;
        }

        unsigned int getWidth() const { return width; }
        unsigned int getHeight() const { return height; }
        unsigned int getDepth() const { return depth; }


    private:
        int width, height, depth;
        std::vector<int> data;

        // Retrieve data of index
        int getIndex(const int x, const int y, const int z) const
        {
            if (x >= width || y >= height || z >= depth) {
                throw std::out_of_range("Grid3D: Index out of bounds");
            }
            return z * (width * height) + y * width + x;
        }
};
