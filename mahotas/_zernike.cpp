#include <complex>
#include <cmath>
#include <new>

extern "C" {
    #include <Python.h>
    #include <numpy/ndarrayobject.h>
}

namespace{

const char TypeErrorMsg[] =
    "Type not understood. "
    "This is caused by either a direct call to _zernike (which is dangerous: types are not checked!) or a bug in zernike.py.\n";

double _factorialtable[] = {
        1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600
};

inline
double fact(int n) {
    if (unsigned(n) < sizeof(_factorialtable)/sizeof(double)) return _factorialtable[n];
    return n * fact(n - 1);
}

PyObject* py_znl(PyObject* self, PyObject* args) {
    using std::pow;
    using std::atan;
    using std::atan2;
    using std::polar;
    using std::conj;
    using std::complex;

    const double pi = atan(1.0)*4;

    PyArrayObject* Da;
    PyArrayObject* Aa;
    PyArrayObject* Pa;
    double n;
    double l;
    if (!PyArg_ParseTuple(args,"OOOdd", &Da, &Aa, &Pa, &n, &l)) return NULL;
    if (!PyArray_Check(Da) || !PyArray_Check(Aa) || !PyArray_Check(Pa) ||
        PyArray_TYPE(Da) != NPY_DOUBLE || PyArray_TYPE(Aa) != NPY_DOUBLE || PyArray_TYPE(Pa) != NPY_DOUBLE) {
        PyErr_SetString(PyExc_RuntimeError, TypeErrorMsg);
        return NULL;
    }
    double* D = static_cast<double*>(PyArray_DATA(Da));
    double* A = static_cast<double*>(PyArray_DATA(Aa));
    double* P = static_cast<double*>(PyArray_DATA(Pa));
    int Nelems = PyArray_SIZE(Da);
    complex<double> Vnl = 0.0;
    complex<double> v = 0.;
    double * g_m = new(std::nothrow) double[ int( (n-l)/2 ) + 1];
    if (!g_m) {
        PyErr_NoMemory();
        return NULL;
    }
    for(int m = 0; m <= (n-l)/2; m++) {
        double f = (m & 1) ? -1 : 1;
        g_m[m] = f * fact(n-m) /
               ( fact(m) * fact((n - 2*m + l) / 2) * fact((n - 2*m - l) / 2) );
    }

    for (int i = 0; i != Nelems; ++i) {
        double d=D[i];
        double a=A[i];
        double p=P[i];
        Vnl = 0.;
        for(int m = 0; m <= (n-l)/2; m++) {
            Vnl += g_m[m] * pow(d, (double)(n - 2*m)) * polar(1.0, l*a);
        }
        v += p * conj(Vnl);
    }
    v *= (n+1)/pi;
    delete [] g_m;
    return PyComplex_FromDoubles(v.real(), v.imag());

}
PyMethodDef methods[] = {
  {"znl",(PyCFunction)py_znl, METH_VARARGS, NULL},
  {NULL, NULL,0,NULL},
};

} // namespace
extern "C"
void init_zernike()
  {
    import_array();
    (void)Py_InitModule("_zernike", methods);
  }

