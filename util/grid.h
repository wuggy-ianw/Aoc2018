//
// Created by wuggy on 22/12/18.
//

#ifndef AOC2018_GRID_H
#define AOC2018_GRID_H

struct Point {
    int x;
    int y;

    Point() : x(0), y(0) {}

    explicit Point(int ax, int ay) : x(ax), y(ay) {}

    Point(const Point &) = default;

    Point(Point &&) = default;

    Point &operator=(const Point &) = default;

    Point &operator=(Point &&) = default;

    bool is_positive()
    {
        return (y>=0 && x>=0);
    }

    // DO NOT include comparators
    // Some puzzles need different relative point orderings
};



template<typename T>
class NoddySparseGrid
{
private:
    const T default_item;
    std::vector<std::vector<T>> grid;
    std::vector<T> dummy_row;   // used for const RowAccessors to out-of-bound rows

    size_t max_col = 0;

public:
    explicit NoddySparseGrid(const T& item = T()) : default_item(item) {}

    NoddySparseGrid(const NoddySparseGrid&) = default;
    NoddySparseGrid(NoddySparseGrid&&) noexcept = default;

    NoddySparseGrid& operator=(const NoddySparseGrid&) = default;
    NoddySparseGrid& operator=(NoddySparseGrid&&) noexcept = default;

    class RowAccessor
    {
    private:
        const T &default_item;
        std::vector<T> &row;
        size_t& max_col;

    public:
        RowAccessor(const T &item, std::vector<T> &r, size_t& mc) : default_item(item), row(r), max_col(mc) {}
        RowAccessor(const RowAccessor &) = default;
        RowAccessor(RowAccessor &&) noexcept = default;

        T &operator[](size_t i)
        {
            max_col = std::max(max_col, i);

            if (i >= row.size()) row.resize(i + 1, default_item);
            return row[i];
        }
    };


    class ConstRowAccessor
    {
    private:
        const T &default_item;
        const std::vector<T> &row;

    public:
        ConstRowAccessor(const T &item, const std::vector<T> &r) : default_item(item), row(r) {}
        ConstRowAccessor(const ConstRowAccessor &) = default;
        ConstRowAccessor(ConstRowAccessor &&) noexcept = default;

        const T& operator[](size_t i) const
        {
            if (i>=row.size()) return default_item;
            return row[i];
        }
    };


    RowAccessor operator[](size_t i)
    {
        if (i>=grid.size()) grid.resize(i + 1);
        return RowAccessor(default_item, grid[i], max_col);
    }

    ConstRowAccessor operator[](size_t i) const
    {
        if (i>=grid.size()) return ConstRowAccessor(default_item, dummy_row);
        return ConstRowAccessor(default_item, grid[i]);
    }

    T& operator[](const Point& p)
    {
        return (*this)[p.y][p.x];
    }

    const T& operator[](const Point& p) const
    {
        return (*this)[p.y][p.x];
    }

    size_t rows() const
    {
        return grid.size();
    }

    size_t columns() const
    {
        return max_col + 1; // max_col is the max filled column
    }

};


#endif //AOC2018_GRID_H
