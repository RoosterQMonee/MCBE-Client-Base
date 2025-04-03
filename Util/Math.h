#pragma once

template<typename T>
class Vector2 {
public:
    T x = 0, y = 0;
public:
    explicit Vector2(T x = 0, T y = 0) {
        this->x = x;
        this->y = y;
    };
public:
    auto sub(const Vector2<T> &v) -> Vector2<T> {
        return Vector2<T>(this->x - v.x, this->y - v.y);
    };

    auto add(const Vector2<T> &v) -> Vector2<T> {
        return Vector2<T>(this->x + v.x, this->y + v.y);
    };

    auto div(const Vector2<T> &v) -> Vector2<T> {
        return Vector2<T>(this->x / v.x, this->y / v.y);
    };

    auto mul(const Vector2<T> &v) -> Vector2<T> {
        return Vector2<T>(this->x * v.x, this->y * v.y);
    };

    auto mul(float v) -> Vector2<T> {
        return Vector2<T>(this->x * v, this->y * v);
    };

    auto dist(const Vector2<T> &v) -> float {
        auto dX = this->x - v.x;
        auto dY = this->y - v.y;

        return sqrt(dX * dX + dY * dY);
    };

    auto ToInt() {
        return Vector2<int>(x, y);
    }

    auto operator+(const Vector2<T> Vec) {
		return Vector2(this->x + Vec.x, this->y + Vec.y);
	}

    auto operator==(const Vector2<T> Vec) {
        return this->x == Vec.x && this->y == Vec.y;
    }

	auto operator-(const Vector2<T> Vec) {
		return Vector2(this->x - Vec.x, this->y - Vec.y);
	}
};

template<typename T>
class Vector3 : public Vector2<T> {
public:
    T z = 0;
public:
    explicit Vector3(T x = 0, T y = 0, T z = 0) : Vector2<T>(x, y) {
        this->z = z;
    };

    auto ToFloat() {
        return Vector3<float>(this->x, this->y, z);
    }

    auto add(T x, T y, T z) -> Vector3<T> {
        return Vector3<T>(this->x + x, this->y + y, this->z + z);
    };

    auto add(const Vector3<T> &vec) -> Vector3<T> {
        return Vector3<T>(this->x + vec.x, this->y + vec.y, this->z + vec.z);
    };

    auto add(T v) -> Vector3<T> {
        return Vector3<T>(this->x + v, this->y + v, this->z + v);
    };

    auto sub(T x, T y, T z) -> Vector3<T> {
        return Vector3<T>(this->x - x, this->y - y, this->z - z);
    };

    auto sub(const Vector3<T>& vec) const -> Vector3<T> {
        return Vector3<T>(this->x - vec.x, this->y - vec.y, this->z - vec.z);
    };

    auto sub(T v) -> Vector3<T> {
        return Vector3<T>(this->x - v, this->y - v, this->z - v);
    };

    auto floor() -> Vector3<T> {
        return Vector3<T>(std::floor(this->x), std::floor(this->y), std::floor(this->z));
    };

    auto div(T x, T y, T z) -> Vector3<T> {
        return Vector3<T>(this->x / x, this->y / y, this->z / z);
    };

    auto div(const Vector3<T> &vec) -> Vector3<T> {
        return Vector3<T>(this->x / vec.x, this->y / vec.y, this->z / vec.z);
    };

    auto div(T v) -> Vector3<T> {
        return Vector3<T>(this->x / v, this->y / v, this->z / v);
    };

    auto mul(T x, T y, T z) -> Vector3<T> {
        return Vector3<T>(this->x * x, this->y * y, this->z * z);
    };

    auto mul(const Vector3<T> &vec) -> Vector3<T> {
        return Vector3<T>(this->x * vec.x, this->y * vec.y, this->z * vec.z);
    };

    auto lerp(const Vector3<T> &vec, T t) -> Vector3<T> {
        return Vector3<T>(std::lerp(this->x, vec.x, t), std::lerp(this->y, vec.y, t), std::lerp(this->z, vec.z, t));
    };

    auto mul(T v) -> Vector3<T> {
        return Vector3<T>(this->x * v, this->y * v, this->z * v);
    };
public:
    auto dist(const Vector3<T> pos) const -> float {
        return sqrt((std::pow(this->x - pos.x, 2)) + (std::pow(this->y - pos.y, 2)) + (std::pow(this->z - pos.z, 2)));
    };
};

template<typename T>
class Vector4 : public Vector3<T> {
public:
    T w = 0;
public:
    explicit Vector4(T x = 0, T y = 0, T z = 0, T w = 0) : Vector3<T>(x, y, z) {
        this->w = w;
    };
};