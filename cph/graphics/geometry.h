#pragma once

namespace cph {
struct T2dPoint {
	u16 x;
	u16 y;

	T2dPoint(const T2dPoint& other) : x(other.x), y(other.y) {
	}
	T2dPoint(u16 ax = 0, u16 ay = 0) : x(ax), y(ay) {}

};

struct TGeometricSize {
	u16 Width;
	u16 Height;
	TGeometricSize(const TGeometricSize& other) : Width(other.Width),
		Height(other.Height) {
	}
	TGeometricSize(u16 aWidth = 0, u16 aHeight = 0) : Width(aWidth),
		Height(aHeight) {}
};

}