/* The MIT License:

Copyright (c) 2008-2012 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// Home page: http://morda.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

//This macro definition is part of a workaround to prevent IDE complain about recursive includes.
#define M_VECTOR4_HPP_INCLUDED

#ifdef DEBUG
#	include <iostream>
#endif

#include <ting/debug.hpp>
#include <ting/math.hpp>



namespace morda{



//forward declarations
template <class T> class Vector2;
template <class T> class Vector3;



/**
 * @brief Four-dimensional vector.
 */
template <class T> class Vector4{
public:
	/**
	 * @brief First vector component.
	 */
	T x;
	
	/**
	 * @brief Second vector component.
	 */
	T y;
	
	/**
	 * @brief Third vector component.
     */
	T z;
	
	/**
	 * @brief Fourth vector component.
     */
	T w;

	/**
	 * @brief Default constructor.
	 * Default constructor does not initialize vector components to any values.
	 */
	Vector4()throw(){}

	/**
	 * @brief Constructor.
	 * Initializes vector components to given values.
     * @param x - value for first vector component.
     * @param y - value for second vector component.
     * @param z - value for third vector component.
     * @param w - value for fourth vector component.
     */
	Vector4(T x, T y, T z, T w)throw() :
			x(x),
			y(y),
			z(z),
			w(w)
	{}

	//NOTE: copy constructor will be generated by compiler

	/**
	 * @brief Constructor.
	 * Initializes all vector components to a given value.
     * @param num - value to initialize all vector components with.
     */
	Vector4(T num)throw(){
		this->operator=(num);
	}

	/**
	 * @brief Constructor.
	 * Initializes first three vector components to the same given value and fourth component to another given value.
     * @param num - value to use for initialization of first three vector components.
     * @param w - value to use for initialization of fourth vector component.
     */
	Vector4(T num, T w)throw() :
			x(num),
			y(num),
			z(num),
			w(w)
	{}

	/**
	 * @brief Constructor.
	 * Initializes components to a given values.
	 * @param vec - 2d vector to use for initialization of first two vector components.
	 * @param z - value to use for initialization of 3rd vector component.
	 * @param w - value to use for initialization of 4th vector component.
	 */
	inline Vector4(const Vector2<T>& vec, T z = 0, T w = 1)throw();

	/**
	 * @brief Constructor.
	 * Initializes components to a given values.
	 * @param vec - 23 vector to use for initialization of first three vector components.
	 * @param w - value to use for initialization of 4th vector component.
	 */
	inline Vector4(const Vector3<T>& vec, T w = 1)throw();

	/**
	 * @brief Access vector component.
     * @param i - component index to access, must be from 0 to 3.
     * @return Reference to the requested vector component.
     */
	T& operator[](unsigned i)throw(){
		ASSERT(i < 4)
		ASSERT( &((&this->x)[0]) == &this->x)
		ASSERT( &((&this->x)[1]) == &this->y)
		ASSERT( &((&this->x)[2]) == &this->z)
		ASSERT( &((&this->x)[3]) == &this->w)
		return (&this->x)[i];
	}

	/**
	 * @brief Access vector component.
	 * Constant version of operator[].
     * @param i - component index to access, must be from 0 to 3.
     * @return constant reference to the requested vector component.
     */
	const T& operator[](unsigned i)const throw(){
		ASSERT(i < 4)
		ASSERT( &((&this->x)[0]) == &this->x)
		ASSERT( &((&this->x)[1]) == &this->y)
		ASSERT( &((&this->x)[2]) == &this->z)
		ASSERT( &((&this->x)[3]) == &this->w)
		return (&this->x)[i];
	}

	//NOTE: operator=() will be generated by compiler

	/**
	 * @brief Assign from 3d vector.
	 * Assigns first 3 components of this vector from components of given 3d vector.
	 * Fourth component of this vector is assigned a value of 1.
	 * @param vec - 3d vector to assign first three components from.
	 * @return Reference to this vector object.
	 */
	inline Vector4& operator=(const Vector3<T>& vec)throw();

	/**
	 * @brief Assign from 2d vector.
	 * Assigns first 2 components of this vector from components of given 2d vector.
	 * Third component of this vector is assigned a value of 0.
	 * Fourth component of this vector is assigned a value of 1.
	 * @param vec - 2d vector to assign first two components from.
	 * @return Reference to this vector object.
	 */
	inline Vector4& operator=(const Vector2<T>& vec)throw();

	/**
	 * @brief Assign a number.
	 * Sets all 4 components of this vector to a given number.
     * @param num - number to use for assignment.
     * @return Reference to this vector object.
     */
	Vector4& operator=(T num)throw(){
		this->x = num;
		this->y = num;
		this->z = num;
		this->w = num;
		return (*this);
	}

	/**
	 * @brief Set all vector components to given value.
	 * @param val - value to set vector components to.
	 * @return Reference to this vector object.
	 */
	Vector4& SetTo(T val)throw(){
		this->x = val;
		this->y = val;
		this->z = val;
		this->w = val;
		return (*this);
	}
	
	/**
	 * @brief Add and assign.
	 * Adds corresponding components of a given vector to first two components of this vector and assigns
	 * the result back to this vector components.
	 * @param vec - 2d vector to use for addition.
	 * @return Reference to this vector object.
	 */
	inline Vector4& operator+=(const Vector2<T>& vec)throw();

	/**
	 * @brief Add and assign.
	 * Adds corresponding components of a given vector to first three components of this vector and assigns
	 * the result back to this vector components.
	 * @param vec - 3d vector to use for addition.
	 * @return Reference to this vector object.
	 */
	inline Vector4& operator+=(const Vector3<T>& vec)throw();

	/**
	 * @brief Add and assign.
	 * Adds given vector to this vector and assigns result back to this vector.
	 * @param vec - vector to add.
	 * @return Reference to this vector object.
	 */
	Vector4& operator+=(const Vector4& vec)throw(){
		this->x += vec.x;
		this->y += vec.y;
		this->z += vec.z;
		this->w += vec.w;
		return *this;
	}

	/**
	 * @brief Add vector.
	 * Adds this vector and given vector.
	 * @param vec - vector to add.
	 * @return Vector resulting from vector addition.
	 */
	Vector4 operator+(const Vector4& vec)const throw(){
		return (Vector4(*this) += vec);
	}

	/**
	 * @brief Subtract and assign.
	 * Subtracts given vector from this vector and assigns result back to this vector.
	 * @param vec - vector to subtract.
	 * @return Reference to this vector object.
	 */
	Vector4& operator-=(const Vector4& vec)throw(){
		this->x -= vec.x;
		this->y -= vec.y;
		this->z -= vec.z;
		this->w -= vec.w;
		return *this;
	}

	/**
	 * @brief Subtract vector.
	 * Subtracts given vector from this vector.
	 * @param vec - vector to subtract.
	 * @return Vector resulting from vector subtraction.
	 */
	Vector4 operator-(const Vector4& vec)const throw(){
		return (Vector4(*this) -= vec);
	}

	/**
	 * @brief Unary minus.
     * @return Negated vector.
     */
	Vector4 operator-()const throw(){
		return Vector4(*this).Negate();
	}

	/**
	 * @brief Multiply by scalar and assign.
	 * Multiplies this vector by scalar and assigns result back to this vector.
     * @param num - scalar to multiply by.
     * @return Reference to this vector object.
     */
	Vector4& operator*=(T num)throw(){
		this->x *= num;
		this->y *= num;
		this->z *= num;
		this->w *= num;
		return (*this);
	}

	/**
	 * @brief Multiply by scalar.
	 * Multiplies this vector by scalar.
     * @param num - scalar to multiply by.
     * @return Vector resulting from multiplication of this vector by scalar.
     */
	Vector4 operator*(T num)const throw(){
		return (Vector4(*this) *= num);
	}

	/**
	 * @brief Multiply scalar by vector.
	 * @param num - scalar to multiply.
	 * @param vec - vector to multiply by.
	 * @return Vector resulting from multiplication of given scalar by given vector.
	 */
	friend Vector4 operator*(T num, const Vector4& vec)throw(){
		return vec * num;
	}

	/**
	 * @brief Divide by scalar and assign.
	 * Divide this vector by scalar and assign result back to this vector.
	 * @param num - scalar to divide by.
	 * @return Reference to this vector object.
	 */
	Vector4& operator/=(T num)throw(){
		ASSERT_INFO(num != 0, "Vector4::operator/=(): division by 0")
		this->x /= num;
		this->y /= num;
		this->z /= num;
		this->w /= num;
		return (*this);
	}

	/**
	 * @brief Divide by scalar.
	 * Divide this vector by scalar.
	 * @param num - scalar to divide by.
	 * @return Vector resulting from division of this vector by scalars.
	 */
	Vector4 operator/(T num)throw(){
		ASSERT_INFO(num != 0, "Vector4::operator/(): division by 0")
		return (Vector4(*this) /= num);
	}

	/**
	 * @brief Dot product.
     * @param vec -vector to multiply by.
     * @return Dot product of this vector and given vector.
     */
	T operator*(const Vector4& vec)const throw(){
		return this->x * vec.x
				+ this->y * vec.y
				+ this->z * vec.z
				+ this->w * vec.w;
	}

	/**
	 * @brief Cross product.
	 * First three components of the resulting 4d vector is a result of cross
	 * product between two 3d vectors formed from first 3 components of initial 4d vectors.
	 * The forth component is a simple multiplication of 4th components of initial vectors.
     * @param vec - vector to multiply by.
     * @return Four-dimensional vector resulting from the cross product.
     */
	Vector4 operator%(const Vector4& vec)const throw(){
		return Vector4(
				this->y * vec.z - this->z * vec.y,
				this->z * vec.x - this->x * vec.z,
				this->x * vec.y - this->y * vec.x,
				this->w * vec.w
			);
	}

	/**
	 * @brief Negate this vector.
	 * Negates this vector.
	 * @return Reference to this vector object.
	 */
	Vector4& Negate()throw(){
		this->x = -this->x;
		this->y = -this->y;
		this->z = -this->z;
		this->w = -this->w;
		return (*this);
	}

	/**
	 * @brief Calculate power 2 of vector magnitude.
	 * @return Power 2 of this vector magnitude.
	 */
	T MagPow2()const throw(){
		return ting::math::Pow2(this->x)
				+ ting::math::Pow2(this->y)
				+ ting::math::Pow2(this->z)
				+ ting::math::Pow2(this->w);
	}

	/**
	 * @brief Calculate vector magnitude.
	 * @return Vector magnitude.
	 */
	T Magnitude()const throw(){
		return ting::math::Sqrt(this->MagPow2());
	}

	/**
	 * @brief Normalize this vector.
	 * Normalizes this vector.
	 * If magnitude is 0 then the result is vector (1, 0, 0, 0).
	 * @return Reference to this vector object.
	 */
	Vector4& Normalize()throw(){
		T mag = this->Magnitude();
		if(mag == 0){
			this->x = 1;
			this->y = 0;
			this->z = 0;
			this->w = 0;
			return *this;
		}
		
		return (*this) /= this->Magnitude();
	}

#ifdef DEBUG
	friend std::ostream& operator<<(std::ostream& s, const Vector4<T>& vec){
		s << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return s;
	}
#endif
};//~class Vector4



}//~namespace



#ifndef M_VECTOR2_HPP_INCLUDED
#	include "Vector2.hpp"
#endif

#ifndef M_VECTOR3_HPP_INCLUDED
#	include "Vector3.hpp"
#endif



namespace morda{

//=================================
// inline functions implementation
//=================================

template <class T> inline Vector4<T>::Vector4(const Vector2<T>& vec, T z, T w)throw() :
		x(vec.x),
		y(vec.y),
		z(z),
		w(w)
{}



template <class T> inline Vector4<T>::Vector4(const Vector3<T>& vec, T w)throw() :
		x(vec.x),
		y(vec.y),
		z(vec.z),
		w(w)
{}



template <class T> inline Vector4<T>& Vector4<T>::operator=(const Vector3<T>& vec)throw(){
	this->x = vec.x;
	this->y = vec.y;
	this->z = vec.z;
	this->w = 1;
	return *this;
}



template <class T> inline Vector4<T>& Vector4<T>::operator=(const Vector2<T>& vec)throw(){
	this->x = vec.x;
	this->y = vec.y;
	this->z = 0;
	this->w = 1;
	return *this;
}



template <class T> inline Vector4<T>& Vector4<T>::operator+=(const Vector2<T>& vec)throw(){
	this->x += vec.x;
	this->y += vec.y;
	return *this;
}



template <class T> inline Vector4<T>& Vector4<T>::operator+=(const Vector3<T>& vec)throw(){
	this->x += vec.x;
	this->y += vec.y;
	this->z += vec.z;
	return *this;
}



//=====================
// Convenient typedefs
//=====================

typedef Vector4<float> Vec4f;
STATIC_ASSERT(sizeof(Vec4f) == sizeof(float) * 4)
typedef Vector4<double> Vec4d;
STATIC_ASSERT(sizeof(Vec4d) == sizeof(double) * 4)



}//~namespace
