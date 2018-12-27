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

    bool is_positive() const
    {
        return (y>=0 && x>=0);
    }

    // DO NOT include comparators
    // Some puzzles need different relative point orderings
};



namespace std
{
    template<> struct hash<Point>
    {
        typedef Point argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& p) const noexcept
        {
            constexpr result_type m = 24251;
            return std::hash<int>()(p.x) + std::hash<int>()(p.y) * m;
        }
    };
}


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

template<typename T>
class OriginCenteredGrid
{
private:
    NoddySparseGrid<T> posx_posy_grid;
    NoddySparseGrid<T> negx_posy_grid;
    NoddySparseGrid<T> posx_negy_grid;
    NoddySparseGrid<T> negx_negy_grid;

public:
    typedef int64_t index_type;

    explicit OriginCenteredGrid(const T& item = T())
        : posx_posy_grid(item), negx_posy_grid(item),
          posx_negy_grid(item), negx_negy_grid(item)
        {}

    OriginCenteredGrid(const OriginCenteredGrid&) = default;
    OriginCenteredGrid(OriginCenteredGrid&&) noexcept = default;

    OriginCenteredGrid& operator=(const OriginCenteredGrid&) = default;
    OriginCenteredGrid& operator=(OriginCenteredGrid&&) noexcept = default;


    class RowAccessor
    {
    private:
        typename NoddySparseGrid<T>::RowAccessor posx_accessor;
        typename NoddySparseGrid<T>::RowAccessor negx_accessor;

    public:
        RowAccessor(typename NoddySparseGrid<T>::RowAccessor&& posx_a, typename NoddySparseGrid<T>::RowAccessor&& negx_a)
            : posx_accessor(posx_a), negx_accessor(negx_a) {}

        RowAccessor(const RowAccessor &) = default;
        RowAccessor(RowAccessor &&) noexcept = default;

        T& operator[](index_type i)
        {
            if (i>=0) return posx_accessor[i];
            else return negx_accessor[-i];
        }
    };

    class ConstRowAccessor
    {
    private:
        typename NoddySparseGrid<T>::ConstRowAccessor posx_accessor;
        typename NoddySparseGrid<T>::ConstRowAccessor negx_accessor;

    public:
        ConstRowAccessor(typename NoddySparseGrid<T>::ConstRowAccessor&& posx_a, typename NoddySparseGrid<T>::ConstRowAccessor&& negx_a)
                : posx_accessor(posx_a), negx_accessor(negx_a) {}

        ConstRowAccessor(const ConstRowAccessor &) = default;
        ConstRowAccessor(ConstRowAccessor &&) noexcept = default;

        const T& operator[](index_type i) const
        {
            if (i>=0) return posx_accessor[i];
            else return negx_accessor[-i];
        }
    };

    RowAccessor operator[](index_type i)
    {
        if (i>=0) return RowAccessor(posx_posy_grid[i], negx_posy_grid[i]);
        else return RowAccessor(posx_negy_grid[-i], negx_negy_grid[-i]);
    }

    ConstRowAccessor operator[](index_type i) const
    {
        if (i>=0) return ConstRowAccessor(posx_posy_grid[i], negx_posy_grid[i]);
        else return ConstRowAccessor(posx_negy_grid[-i], negx_negy_grid[-i]);
    }

    T& operator[](const Point& p)
    {
        return (*this)[p.y][p.x];
    }

    const T& operator[](const Point& p) const
    {
        return (*this)[p.y][p.x];
    }

    index_type min_row() const  // inclusive!
    {
        return -(std::max(posx_negy_grid.rows(), negx_negy_grid.rows()) - 1);
    }

    index_type max_row() const  // inclusive!
    {
        return std::max(posx_posy_grid.rows(), negx_posy_grid.rows()) - 1;
    }

    index_type min_column() const  // inclusive!
    {
        return -(std::max(negx_posy_grid.columns(), negx_negy_grid.columns()) - 1);
    }

    index_type max_column() const  // inclusive!
    {
        return std::max(posx_posy_grid.columns(), posx_negy_grid.columns()) - 1;
    }


};


#endif //AOC2018_GRID_H
