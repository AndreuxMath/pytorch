#include "DynamicTypes.h"

#include "THP.h"
#include <vector>
#include <unordered_map>
#include <THPP/tensors/THTensor.hpp>
#include <THPP/tensors/THSTensor.hpp>

#ifdef WITH_CUDA
#include <THC/THC.h>
#include <THCS/THCS.h>
#include <THPP/tensors/THCTensor.hpp>
#include <THPP/tensors/THCSTensor.hpp>
extern THCState *state;
#endif

using namespace thpp;

namespace torch {

struct TensorType {
  Type data_type;
  bool is_cuda;
  bool is_sparse;

  friend bool operator==(const TensorType &t1, const TensorType &t2) {
    return (t1.data_type == t2.data_type && t1.is_cuda == t2.is_cuda &&
            t1.is_sparse == t2.is_sparse);
  }

  friend bool operator!=(const TensorType &t1, const TensorType &t2) {
    return !(t1 == t2);
  }
};

struct TensorTypeHasher {
  std::size_t operator()(const TensorType &k) const {
    size_t hash = static_cast<size_t>(k.data_type);
    hash = (hash << 8) + k.is_cuda;
    hash = (hash << 1) + k.is_sparse;
    return hash;
  }
};

static std::unordered_map<std::string, Type> type_names = {
    {"Float", Type::FLOAT},   {"Double", Type::DOUBLE},
    {"ZFloat", Type::ZFLOAT}, {"ZDouble", Type::ZDOUBLE},
    {"Half", Type::HALF},     {"Byte", Type::UCHAR},
    {"Char", Type::CHAR},     {"Short", Type::SHORT},
    {"Int", Type::INT},       {"Long", Type::LONG},
};
static std::unordered_map<PyTypeObject *, TensorType> pytype_to_tensortype;
static std::unordered_map<TensorType, PyTypeObject *, TensorTypeHasher>
    tensortype_to_pytype;

void registerPyTypeObject(PyTypeObject *pytype, const std::string &name,
                          bool is_cuda, bool is_sparse) {
  TensorType type;
  type.data_type = type_names.at(name);
  type.is_cuda = is_cuda;
  type.is_sparse = is_sparse;

  pytype_to_tensortype[pytype] = type;
  tensortype_to_pytype[type] = pytype;
}

PyTypeObject *getPyTypeObject(const thpp::Tensor &tensor) {
  TensorType type;
  type.data_type = tensor.type();
  type.is_cuda = tensor.isCuda();
  type.is_sparse = tensor.isSparse();

  return tensortype_to_pytype.at(type);
}

static std::unique_ptr<Tensor> createTensor(void *tensor, Type type,
                                            bool is_cuda, bool is_sparse) {
  if (is_cuda) {
#ifdef WITH_CUDA
    if (is_sparse) {
      if (type == Type::UCHAR) {
        return std::unique_ptr<Tensor>(
            new THCSTensor<unsigned char>(state, (THCSByteTensor *)tensor));
      } else if (type == Type::CHAR) {
        return std::unique_ptr<Tensor>(
            new THCSTensor<char>(state, (THCSCharTensor *)tensor));
      } else if (type == Type::SHORT) {
        return std::unique_ptr<Tensor>(
            new THCSTensor<short>(state, (THCSShortTensor *)tensor));
      } else if (type == Type::INT) {
        return std::unique_ptr<Tensor>(
            new THCSTensor<int>(state, (THCSIntTensor *)tensor));
      } else if (type == Type::LONG) {
        return std::unique_ptr<Tensor>(
            new THCSTensor<long>(state, (THCSLongTensor *)tensor));
      } else if (type == Type::FLOAT) {
        return std::unique_ptr<Tensor>(
            new THCSTensor<float>(state, (THCSFloatTensor *)tensor));
      } else if (type == Type::DOUBLE) {
        return std::unique_ptr<Tensor>(
            new THCSTensor<double>(state, (THCSDoubleTensor *)tensor));
      } else if (type == Type::HALF) {
        return std::unique_ptr<Tensor>(
            new THCSTensor<half>(state, (THCSHalfTensor *)tensor));
      }
    } else if (type == Type::UCHAR) {
      return std::unique_ptr<Tensor>(
          new THCTensor<unsigned char>(state, (THCudaByteTensor *)tensor));
    } else if (type == Type::CHAR) {
      return std::unique_ptr<Tensor>(
          new THCTensor<char>(state, (THCudaCharTensor *)tensor));
    } else if (type == Type::SHORT) {
      return std::unique_ptr<Tensor>(
          new THCTensor<short>(state, (THCudaShortTensor *)tensor));
    } else if (type == Type::INT) {
      return std::unique_ptr<Tensor>(
          new THCTensor<int>(state, (THCudaIntTensor *)tensor));
    } else if (type == Type::LONG) {
      return std::unique_ptr<Tensor>(
          new THCTensor<long>(state, (THCudaLongTensor *)tensor));
    } else if (type == Type::FLOAT) {
      return std::unique_ptr<Tensor>(
          new THCTensor<float>(state, (THCudaTensor *)tensor));
    } else if (type == Type::DOUBLE) {
      return std::unique_ptr<Tensor>(
          new THCTensor<double>(state, (THCudaDoubleTensor *)tensor));
    } else if (type == Type::ZFLOAT) {
      return std::unique_ptr<Tensor>(
          new THCTensor<ccx>(state, (THCudaZFloatTensor *)tensor));
    } else if (type == Type::ZDOUBLE) {
      return std::unique_ptr<Tensor>(
          new THCTensor<zcx>(state, (THCudaZDoubleTensor *)tensor));
    } else if (type == Type::HALF) {
      return std::unique_ptr<Tensor>(
          new THCTensor<half>(state, (THCudaHalfTensor *)tensor));
    }
#else
    throw std::runtime_error("Compiled without CUDA support");
#endif
  } else if (is_sparse) {
    if (type == Type::UCHAR) {
      return std::unique_ptr<Tensor>(
          new THSTensor<unsigned char>((THSByteTensor *)tensor));
    } else if (type == Type::CHAR) {
      return std::unique_ptr<Tensor>(
          new THSTensor<char>((THSCharTensor *)tensor));
    } else if (type == Type::SHORT) {
      return std::unique_ptr<Tensor>(
          new THSTensor<short>((THSShortTensor *)tensor));
    } else if (type == Type::INT) {
      return std::unique_ptr<Tensor>(
          new THSTensor<int>((THSIntTensor *)tensor));
    } else if (type == Type::LONG) {
      return std::unique_ptr<Tensor>(
          new THSTensor<long>((THSLongTensor *)tensor));
    } else if (type == Type::FLOAT) {
      return std::unique_ptr<Tensor>(
          new THSTensor<float>((THSFloatTensor *)tensor));
    } else if (type == Type::DOUBLE) {
      return std::unique_ptr<Tensor>(
          new THSTensor<double>((THSDoubleTensor *)tensor));
    }
  } else if (type == Type::UCHAR) {
    return std::unique_ptr<Tensor>(
        new THTensor<unsigned char>((THByteTensor *)tensor));
  } else if (type == Type::CHAR) {
    return std::unique_ptr<Tensor>(new THTensor<char>((THCharTensor *)tensor));
  } else if (type == Type::SHORT) {
    return std::unique_ptr<Tensor>(
        new THTensor<short>((THShortTensor *)tensor));
  } else if (type == Type::INT) {
    return std::unique_ptr<Tensor>(new THTensor<int>((THIntTensor *)tensor));
  } else if (type == Type::LONG) {
    return std::unique_ptr<Tensor>(new THTensor<long>((THLongTensor *)tensor));
  } else if (type == Type::FLOAT) {
    return std::unique_ptr<Tensor>(
        new THTensor<float>((THFloatTensor *)tensor));
  } else if (type == Type::DOUBLE) {
    return std::unique_ptr<Tensor>(
        new THTensor<double>((THDoubleTensor *)tensor));
  } else if (type == Type::ZFLOAT) {
    return std::unique_ptr<Tensor>(
        new THTensor<float _Complex>((THZFloatTensor *)tensor));
  } else if (type == Type::ZDOUBLE) {
    return std::unique_ptr<Tensor>(
        new THTensor<double _Complex>((THZDoubleTensor *)tensor));
  }
  throw std::invalid_argument("Unsupported tensor type");
}

std::unique_ptr<Tensor> createTensor(PyObject *data) {
  auto tensor_type = pytype_to_tensortype.at(Py_TYPE(data));
  auto type = tensor_type.data_type;
  auto tensor = ((THPVoidTensor *)data)->cdata;
  auto wrapper =
      createTensor(tensor, type, tensor_type.is_cuda, tensor_type.is_sparse);
  wrapper->retain();
  return wrapper;
}

PyObject *createPyObject(const thpp::Tensor &tensor) {
  auto type = getPyTypeObject(tensor);
  PyObject *obj = type->tp_alloc(type, 0);
  if (obj) {
    ((THPVoidTensor *)obj)->cdata =
        (THVoidTensor *)const_cast<thpp::Tensor &>(tensor).retain().cdata();
  }
  return obj;
}

} // namespace
