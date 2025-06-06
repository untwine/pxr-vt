# Copyright 2016 Pixar
#
# Licensed under the terms set forth in the LICENSE.txt file available at
# https://openusd.org/license.
#
# pylint: disable=range-builtin-not-iterating
#
# Modified by Jeremy Retailleau.

from __future__ import division

import array
import unittest
import sys, math
from pxr import Gf, Vt

class TestVtArray(unittest.TestCase):

    def test_Constructors(self):
        self.assertTrue(isinstance(Vt.DoubleArray(), Vt.DoubleArray))
        self.assertTrue((len(Vt.DoubleArray()) == 0 and
                len(Vt.DoubleArray(3)) == 3))

        # constructor failures:
        with self.assertRaises(TypeError):
            Vt.DoubleArray((), (1,2,3))
        with self.assertRaises(TypeError):
            Vt.DoubleArray((1,2), (1,2,3))
        with self.assertRaises(TypeError):
            Vt.DoubleArray(('foo', 'bar'), (1,2,3))

        d = Vt.DoubleArray(10)
        d[...] = range(10)
        d = d[::-1]
        self.assertIsInstance(d, Vt.DoubleArray)
        for i in range(10):
            self.assertEqual(d[i], 10 - i - 1)

        self.assertEqual(d[1:2][0], 8)

    def test_BogusIndex(self):
        d = Vt.DoubleArray(4)
        with self.assertRaises(TypeError):
            a = d['foo']

    def test_NegativeIndexes(self):
        d = Vt.DoubleArray([1.0, 3.0, 5.0])
        d[-1] = 2.0
        d[-2] = 4.0
        self.assertEqual(d, [1.0, 4.0, 2.0])

        # Indexing off-the-end with negative indexes should raise.
        with self.assertRaises(IndexError):
            d[-4] = 6.0

        with self.assertRaises(IndexError):
            x = d[-4]

    def test_TooFewElements(self):
        d = Vt.DoubleArray(4)
        with self.assertRaises(ValueError):
            d[...] = range(3)

        d = Vt.DoubleArray(4)
        with self.assertRaises(ValueError):
            d[...] = Vt.DoubleArray(2)

    def test_ZeroElements(self):
        d = Vt.DoubleArray(4)
        with self.assertRaises(ValueError):
            d[...] = range(0)

        d = Vt.DoubleArray(4)
        with self.assertRaises(ValueError):
            d[...] = Vt.DoubleArray()

    def test_WrongType(self):
        d = Vt.DoubleArray(4)
        with self.assertRaises(TypeError):
            d[...] = ('foo',)*4

        # set with scalar of wrong type.
        with self.assertRaises(ValueError):
            # Not enough iterable elements.
            d[...] = 'foo'

        # Type mismatch.
        with self.assertRaises((ValueError,TypeError)):
            d[...] = {'food'}

    def test_ZeroStep(self):
        d = Vt.DoubleArray(4)
        with self.assertRaises(IndexError):
            d[::0] = 10

    def test_SetWithScalar(self):
        d3 = Vt.DoubleArray(2)
        d3[...] = 0

    def test_VtArraySlicingVsPythonSlicing(self):
        '''Validate VtArray slicing against python list slicing for all
           particular index combinations of one dimensional length 4 arrays.'''
        a = Vt.IntArray(4)
        a[...] = range(4)
        l = range(4)
        for start in range(-2, 7):
            for stop in range(-2, 7):
                for step in range(-5, 6):
                    if step == 0 : break
                    sub_a = a[start:stop:step]
                    sub_l = l[start:stop:step]
                    self.assertEqual(len(a), len(l))
                    for i in range(len(a)):
                        self.assertEqual(a[i], l[i])


    def test_Str(self):
        self.assertTrue(len(str(Vt.DoubleArray(3))))
        self.assertEqual(str(Vt.DoubleArray()), '[]')

    def test_Repr(self):
        ia = Vt.IntArray()
        self.assertEqual(ia, eval(repr(ia)))

        sa = Vt.UInt64Array((0, 18446744073709551615))
        self.assertEqual(sa, eval(repr(sa)))

        sa = Vt.StringArray(('\\',))
        self.assertEqual(sa, eval(repr(sa)))

        ba = Vt.BoolArray((True, False))
        self.assertEqual(ba, eval(repr(ba)))

        fa = Vt.FloatArray((1.0000001, 1e39, float('-inf')))
        self.assertGreater(fa[0], 1.0)
        self.assertEqual(fa, eval(repr(fa)))

        da = Vt.DoubleArray((1.0000000000000002, 1e39, 1e309, float('-inf')))
        self.assertGreater(da[0], 1.0)
        self.assertEqual(da, eval(repr(da)))

        ha = Vt.HalfArray((1.0001, 1e39, float('-inf')))
        self.assertGreater(da[0], 1.0)
        self.assertEqual(ha, eval(repr(ha)))

    def test_Overflows(self):
        overflows = [
            # (array type, (largest negative number to overflow,
            #               smallest positive number to overflow))
            (Vt.UCharArray, (-1, 256)),
            (Vt.UShortArray, (-1, 65536)),
            (Vt.UIntArray, (-1, 4294967296)),
            (Vt.UInt64Array, (-1, 18446744073709551616)),
            (Vt.ShortArray, (-32769, 32768)),
            (Vt.IntArray, (-2147483649, 2147483648)),
            (Vt.Int64Array, (-9223372036854775809, 9223372036854775808)),
        ]

        for arrayType, (x0, x1) in overflows:
            for x in (x0, x1):
                with self.assertRaises(OverflowError):
                    arrayT = arrayType((x,))
            for x in (x0+1, x1-1):
                arrayT = arrayType((x,))
                self.assertEqual(arrayT, eval(repr(arrayT)))

    def test_ParallelizedOps(self):
        m0 = Vt.Matrix4dArray((Gf.Matrix4d(1),Gf.Matrix4d(2)))
        m1 = Vt.Matrix4dArray((Gf.Matrix4d(2),Gf.Matrix4d(4)))
        self.assertEqual(m0 * 2, m1)
        self.assertEqual(0.5 * m1, m0)
        self.assertEqual(m1 - m0, m0)
        self.assertEqual(-m0 + m0, Vt.Matrix4dArray((Gf.Matrix4d(0),Gf.Matrix4d(0))))
        self.assertEqual(-m0 + m1, m0)

    def test_Numpy(self):
        '''Converting VtArrays to numpy arrays and back.'''

        try:
            import numpy
        except ImportError:
            # If we don't have numpy, just skip this test.
            return

        cases = [
            dict(length=33, fill=(1,2,3),
                arrayType=Vt.Vec3hArray, expLen=11, expVal=Gf.Vec3h(1,2,3)),
            dict(length=33, fill=(1,2,3),
                arrayType=Vt.Vec3fArray, expLen=11, expVal=Gf.Vec3f(1,2,3)),
            dict(length=33, fill=(1,2,3),
                arrayType=Vt.Vec3dArray, expLen=11, expVal=Gf.Vec3d(1,2,3)),
            dict(length=12, fill=(1,2,3,4),
                arrayType=Vt.Vec4dArray, expLen=3, expVal=Gf.Vec4d(1,2,3,4)),
            dict(length=12, fill=(1,2,3,4),
                arrayType=Vt.QuatdArray, expLen=3, expVal=Gf.Quatd(4, (1,2,3))),
            dict(length=12, fill=(1,2,3,4),
                arrayType=Vt.QuatfArray, expLen=3, expVal=Gf.Quatf(4, (1,2,3))),
            dict(length=12, fill=(1,2,3,4),
                arrayType=Vt.QuathArray, expLen=3, expVal=Gf.Quath(4, (1,2,3))),
            dict(length=48, fill=(1,2,3,4,5,6,7,8),
                arrayType=Vt.DualQuatdArray, expLen=6,
                expVal=Gf.DualQuatd(Gf.Quatd(4, (1,2,3)), Gf.Quatd(8, (5,6,7)))),
            dict(length=48, fill=(1,2,3,4,5,6,7,8),
                arrayType=Vt.DualQuatfArray, expLen=6,
                expVal=Gf.DualQuatf(Gf.Quatf(4, (1,2,3)), Gf.Quatf(8, (5,6,7)))),
            dict(length=48, fill=(1,2,3,4,5,6,7,8),
                arrayType=Vt.DualQuathArray, expLen=6,
                expVal=Gf.DualQuath(Gf.Quath(4, (1,2,3)), Gf.Quath(8, (5,6,7)))),
            dict(length=40, fill=(1,0,0,1),
                arrayType=Vt.Matrix2fArray, expLen=10, expVal=Gf.Matrix2f()),
            dict(length=40, fill=(1,0,0,1),
                arrayType=Vt.Matrix2dArray, expLen=10, expVal=Gf.Matrix2d()),
            dict(length=90, fill=(1,0,0,0,1,0,0,0,1),
                arrayType=Vt.Matrix3fArray, expLen=10, expVal=Gf.Matrix3f()),
            dict(length=90, fill=(1,0,0,0,1,0,0,0,1),
                arrayType=Vt.Matrix3dArray, expLen=10, expVal=Gf.Matrix3d()),
            dict(length=160, fill=(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
                arrayType=Vt.Matrix4fArray, expLen=10, expVal=Gf.Matrix4f()),
            dict(length=160, fill=(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),
                arrayType=Vt.Matrix4dArray, expLen=10, expVal=Gf.Matrix4d()),
            dict(length=24, fill=(4.25,8.5),
                arrayType=Vt.Range1dArray, expLen=12, expVal=Gf.Range1d(4.25,8.5)),
            dict(length=24, fill=(4.25,8.5),
                arrayType=Vt.Range1dArray, expLen=12, expVal=Gf.Range1d(4.25,8.5)),
            dict(length=24, fill=(-1, -1, 1, 1),
                arrayType=Vt.Range2dArray, expLen=6,
                expVal=Gf.Range2d((-1, -1), (1,1)),)
            ]
        for case in cases:
            Array, length, fill, expLen, expVal = (
                case['arrayType'], case['length'], case['fill'],
                case['expLen'], case['expVal'])
            src = Vt.DoubleArray(length, fill)
            result = Array.FromNumpy(numpy.array(src, copy=False))
            self.assertEqual(len(list(result)), expLen)
            self.assertTrue(all([x == expVal for x in result]), \
                '%s != %s' % (list(result), [expVal]*len(list(result))))

        # Formerly failed, now produces a 1-d length-1 array.
        self.assertTrue(Vt.Vec3dArray.FromNumpy(
            numpy.array([1,2,3])) == Vt.Vec3dArray([(1,2,3)]))

        with self.assertRaises((ValueError, TypeError)):
            # Two dimensional, but second dimension only 2 (needs 3).
            a = numpy.array([[1,2],[3,4]])
            v = Vt.Vec3dArray(a)

        # Some special-case empty array cases.
        self.assertEqual(numpy.array(Vt.DoubleArray()).shape, (0,))
        self.assertEqual(numpy.array(Vt.Vec4fArray()).shape, (0, 4))
        self.assertEqual(numpy.array(Vt.Matrix2dArray()).shape, (0, 2, 2))
        self.assertEqual(numpy.array(Vt.Matrix2fArray()).shape, (0, 2, 2))
        self.assertEqual(numpy.array(Vt.Matrix3dArray()).shape, (0, 3, 3))
        self.assertEqual(numpy.array(Vt.Matrix3fArray()).shape, (0, 3, 3))
        self.assertEqual(numpy.array(Vt.Matrix4dArray()).shape, (0, 4, 4))
        self.assertEqual(numpy.array(Vt.Matrix4fArray()).shape, (0, 4, 4))
        for C in (Vt.DoubleArray, Vt.Vec4fArray,
                Vt.Matrix3dArray, Vt.Matrix4dArray):
            self.assertEqual(C(numpy.array(C())), C())

        # Support non-contiguous numpy arrays -- slicing numpy arrays is a
        # convenient way to get these.
        self.assertEqual(Vt.FloatArray.FromNumpy(
            numpy.array(Vt.FloatArray(range(33)))[::4]),
                Vt.FloatArray(9, (0, 4, 8, 12, 16, 20, 24, 28, 32)))
        self.assertEqual(Vt.Vec3dArray.FromNumpy(
            numpy.array(Vt.Vec3fArray([(1,2,3),(4,5,6),(7,8,9)]))[::-2]),
                Vt.Vec3dArray([(7,8,9),(1,2,3)]))

        # Check that python sequences will convert to arrays as well.
        self.assertEqual(Vt.DoubleArray([1,2,3,4]), Vt.DoubleArray((1, 2, 3, 4)))

        # Formerly failed, now works, producing a linear Vec4dArray.
        self.assertEqual(Vt.Vec4dArray.FromNumpy(
            numpy.array([1,2,3,4])), Vt.Vec4dArray([(1,2,3,4)]))

        # Simple 1-d double array.
        da = Vt.DoubleArray(10, range(10))
        self.assertEqual(Vt.DoubleArray(numpy.array(da)), da)

        # Vec3dArray.
        va = Vt.Vec3dArray(10, [(1,2,3),(2,3,4)])
        self.assertEqual(Vt.Vec3dArray.FromNumpy(numpy.array(va)), va)

    def test_IntArrayDivide(self):
        self.assertEqual(Vt.IntArray() / 5, Vt.IntArray())
        self.assertEqual(5 / Vt.IntArray(), Vt.IntArray())

        self.assertEqual(Vt.IntArray([1, 5, 10]) / 5, 
                         Vt.IntArray([0, 1, 2]))
        self.assertEqual(5 / Vt.IntArray([1, 5, 10]),
                         Vt.IntArray([5, 1, 0]))

        a = Vt.IntArray([1, 5, 10])
        a /= 5
        self.assertEqual(a, Vt.IntArray([0, 1, 2]))

        # Can't divide Vt.IntArray by floating point numbers.
        with self.assertRaises(TypeError):
            a /= 5.0

        with self.assertRaises(TypeError):
            Vt.IntArray([1, 2, 3]) / 5.0

    def test_FloatArrayDivide(self):
        def _TestDivision(ArrayType):
            self.assertEqual(ArrayType() / 5, ArrayType())
            self.assertEqual(5 / ArrayType(), ArrayType())

            self.assertEqual(ArrayType([1, 5, 10]) / 5,
                             ArrayType([0.2, 1, 2]))

            self.assertEqual(ArrayType([1, 5, 10]) / 5.0,
                             ArrayType([0.2, 1, 2]))

            self.assertEqual(5 / ArrayType([1, 5, 10]),
                             ArrayType([5, 1, 0.5]))

            self.assertEqual(5.0 / ArrayType([1, 5, 10]),
                             ArrayType([5, 1, 0.5]))

            a = ArrayType([1, 5, 10])
            a /= 5
            self.assertEqual(a, ArrayType([0.2, 1, 2]))

            a = ArrayType([1, 5, 10])
            a /= 5.0
            self.assertEqual(a, ArrayType([0.2, 1, 2]))

        _TestDivision(Vt.FloatArray)
        _TestDivision(Vt.DoubleArray)

    def test_QuatArrayDivision(self):
        def _TestDivision(ArrayType, QuatType, ImgType):
            self.assertEqual(ArrayType() / 5, ArrayType())

            self.assertEqual(
                ArrayType([QuatType(0, ImgType(i)) for i in [1, 5, 10]]) / 5,
                ArrayType([QuatType(0, ImgType(i)) for i in [0.2, 1, 2]]))

            self.assertEqual(
                ArrayType([QuatType(0, ImgType(i)) for i in [1, 5, 10]]) / 5.0,
                ArrayType([QuatType(0, ImgType(i)) for i in [0.2, 1, 2]]))

            a = ArrayType([QuatType(0, ImgType(i)) for i in [1, 5, 10]])
            a /= 5
            self.assertEqual(
                a, 
                ArrayType([QuatType(0, ImgType(i)) for i in [0.2, 1, 2]]))

            a = ArrayType([QuatType(0, ImgType(i)) for i in [1, 5, 10]])
            a /= 5.0
            self.assertEqual(
                a, 
                ArrayType([QuatType(0, ImgType(i)) for i in [0.2, 1, 2]]))

        _TestDivision(Vt.QuathArray, Gf.Quath, Gf.Vec3h)
        _TestDivision(Vt.QuatfArray, Gf.Quatf, Gf.Vec3f)
        _TestDivision(Vt.QuatdArray, Gf.Quatd, Gf.Vec3d)
        _TestDivision(Vt.QuaternionArray, Gf.Quaternion, Gf.Vec3d)

    def test_LargeBuffer(self):
        '''VtArray can be created from a buffer with item count
           greater than maxint'''
        largePyBuffer = array.array('B', (0,)) * 2500000000
        vtArrayFromBuffer = Vt.UCharArray.FromBuffer(largePyBuffer)
        self.assertEqual(len(vtArrayFromBuffer), 2500000000)


if __name__ == '__main__':
    unittest.main()

