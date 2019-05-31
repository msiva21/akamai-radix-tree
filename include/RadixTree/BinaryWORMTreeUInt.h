#ifndef AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_
#define AKAMAI_MAPPER_RADIX_TREE_BINARY_WORM_TREE_UINT_H_

/*
Copyright (c) 2019 Akamai Technologies, Inc

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdint.h>
#include <cstddef>
#include <string>
#include <array>
#include <type_traits>

#include "CursorIterator.h"
#include "MetaUtils.h"

#include "BinaryWORMTree.h"
#include "BinaryWORMTreeBuilder.h"
#include "BinaryWORMTreeGeneric.h"

/**
 * \file BinaryWORMTreeUInt.h
 * 
 * An expected extremely common use case for the binary WORM tree
 * is to store unsigned integers. The generic interface eases the use
 * of the WORM tree format for compact serialization. It becomes much
 * easier to deal with WORM trees that use different numbers of bytes
 * to represent offsets and values.
 */

namespace Akamai {
namespace Mapper {
namespace RadixTree {

/**
 * \brief Contains the size/value/endian parameters for a Binary WORM tree.
 */
struct BinaryWORMTreeUIntParams {
  std::size_t offsetSize{0}; ///< Size in bytes of offset unsigned integer.
  std::size_t valueSize{0}; ///< Size in bytes of value unsigned integer.
  bool isLittleEndian{false}; ///< Whether the offset and value are little endian.
};

/**
 * \brief Tree wrapper specialized for UInt value WORM trees.
 * 
 * Stores underlying properties of the tree buffer as metadata.
 */
template <typename PathT>
class BinaryWORMTreeUIntGeneric
  : public BinaryWORMTreeGeneric<PathT,uint64_t>
{
public:
  using GenericTreeBase = BinaryWORMTreeGeneric<PathT,uint64_t>;
  using TreeImplType = typename GenericTreeBase::TreeImplType;
  BinaryWORMTreeUIntGeneric() = default;
  BinaryWORMTreeUIntGeneric(const BinaryWORMTreeUIntParams& tp,std::unique_ptr<TreeImplType>&& t)
    : GenericTreeBase(std::move(t))
    , treeParams_(tp)
  {}
  virtual ~BinaryWORMTreeUIntGeneric() = default;
  const BinaryWORMTreeUIntParams& treeParams() const { return treeParams_; }

private:
  BinaryWORMTreeUIntParams treeParams_{};
};

template <bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMNodeUIntWO = BinaryWORMNodeWO<OFFSETSIZE,LITTLEENDIAN,BinaryWORMReadWriteUInt<VALUESIZE,LITTLEENDIAN>>;

template <bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMNodeUIntRO = BinaryWORMNodeRO<OFFSETSIZE,LITTLEENDIAN,BinaryWORMReadWriteUInt<VALUESIZE,LITTLEENDIAN>>;

template <typename BufferT,typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMTreeUIntBuilder = BinaryWORMTreeBuilder<BufferT,PathT,BinaryWORMNodeUIntWO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename BufferT,typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMTreeUInt = BinaryWORMTree<BufferT,PathT,BinaryWORMNodeUIntRO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

/**
 * \brief Generic UINT cursor type, uses uint64_t as value so that up to 8 bytes per value integer may be used.
 */
template <typename PathT>
using BinaryWORMCursorUIntGeneric = BinaryWORMCursorROGeneric<PathT,uint64_t>;

/**
 * \brief Wraps an actual WORM UINT cursor instance inside of the generic cursor interface.
 */
template <typename ActualImplT>
class BinaryWORMCursorUIntGenericImpl
  : public BinaryWORMCursorROGenericImpl<typename ActualImplT::PathType,uint64_t>
{
public:
  using ValueType = uint64_t;  
  using PathType = typename ActualImplT::PathType;
  using GenericImpl = BinaryWORMCursorROGenericImpl<PathType,uint64_t>;
  BinaryWORMCursorUIntGenericImpl() = default;
  template <typename... ConstructorArgs>
  BinaryWORMCursorUIntGenericImpl(ConstructorArgs&&... cargs) : actualCursor_(std::forward<ConstructorArgs>(cargs)...) {}
  virtual ~BinaryWORMCursorUIntGenericImpl() = default;

  virtual bool atNode() const override { return actualCursor_.atNode(); }
  virtual bool atLeafNode() const override { return actualCursor_.atLeafNode(); }
  virtual bool atValue() const override { return actualCursor_.atValue(); }
  virtual bool goChild(std::size_t child) override { return actualCursor_.goChild(child); }
  virtual bool canGoChild(std::size_t child) const override { return actualCursor_.canGoChild(child); }
  virtual bool canGoChildNode(std::size_t child) const override { return actualCursor_.canGoChildNode(child); }
  virtual bool goParent() override { return actualCursor_.goParent(); }
  virtual bool canGoParent() const override { return actualCursor_.canGoParent(); }
  virtual PathType getPath() const override { return actualCursor_.getPath(); }
  virtual ValueType valueCopy() const override { return static_cast<uint64_t>(*(actualCursor_.nodeValueRO().getPtrRO())); }
  virtual GenericImpl* copy() const override { return new BinaryWORMCursorUIntGenericImpl<ActualImplT>(actualCursor_); }

  const ActualImplT& actualCursor() const { return actualCursor_; }
  ActualImplT& actualCursor() { return actualCursor_; }

private:
  ActualImplT actualCursor_{};
};

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMCursorUInt = BinaryWORMCursorRO<PathT,BinaryWORMNodeUIntRO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>,SimpleFixedDepthStack>;

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMLookupCursorUInt = BinaryWORMLookupCursorRO<PathT,BinaryWORMNodeUIntRO<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMCursorUIntImpl = BinaryWORMCursorUIntGenericImpl<BinaryWORMCursorUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
using BinaryWORMLookupCursorUIntImpl = BinaryWORMCursorUIntGenericImpl<BinaryWORMLookupCursorUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>>;

/**
 * \brief Wraps WORM UINT tree implementation in generic wrapper.
 */
template <typename BufferT,typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE>
class BinaryWORMTreeUIntGenericImpl
  : public BinaryWORMTreeGenericImpl<PathT,uint64_t>
{
public:
  using ActualImpl = BinaryWORMTreeUInt<BufferT,PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using GenericImpl = BinaryWORMTreeGenericImpl<PathT,uint64_t>;
  using GenericCursor = BinaryWORMCursorUIntGeneric<PathT>;
  using ActualWalkCursorImpl = BinaryWORMCursorUIntImpl<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using ActualLookupCursorImpl = BinaryWORMLookupCursorUIntImpl<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using CursorROType = typename GenericImpl::CursorROType;
  using LookupCursorROType = typename GenericImpl::LookupCursorROType;

  template <typename... ConstructorArgs>
  BinaryWORMTreeUIntGenericImpl(ConstructorArgs&&... cargs) : actualTree_(std::forward<ConstructorArgs>(cargs)...) {}
  
  virtual CursorROType walkCursorRO() const override {
    std::unique_ptr<ActualWalkCursorImpl> cursorImpl(new ActualWalkCursorImpl(actualTree_.cursorRO()));
    return CursorROType(std::move(cursorImpl));
  }
  virtual LookupCursorROType lookupCursorRO() const override {
    std::unique_ptr<ActualLookupCursorImpl> lookupCursorImpl(new ActualLookupCursorImpl(actualTree_.lookupCursorRO()));
    return LookupCursorROType(std::move(lookupCursorImpl));
  }

  virtual const uint8_t* bytes() const override {
    return actualTree_.buffer().data();
  }

  virtual std::size_t bytesSize() const override {
    return actualTree_.buffer().size();
  }
private:
  ActualImpl actualTree_{};
};

/**
 * \brief Pre-order traverse cursor doing a dry-run WORM tree build to find minimum offset and value sizes.
 *
 * The cursor should cover a tree that contains unsinged integers, uint64_t or smaller.
 * The tree parameters derived here can be fed directly into buildWORMTreeUIntGeneric
 * to create a treee.
 */
template <typename CursorT>
inline
BinaryWORMTreeUIntParams
findMinimumWORMTreeUIntParameters(const CursorT& c);

/**
 * \brief Pre-order traverse cursor, build a WORM tree using endian/value/offset sizes in treeParams.
 */
template <typename CursorT,typename BufferT = std::vector<uint8_t>>
inline
BinaryWORMTreeUIntGeneric<typename std::decay<CursorT>::type::PathType>
buildWORMTreeUIntGeneric(const BinaryWORMTreeUIntParams& treeParams,const CursorT& sourceCursor);

/**
 * \brief Take buffer, create a generic WORM tree using parameters in treeParams.
 */
template <typename PathT,typename BufferT>
inline
BinaryWORMTreeUIntGeneric<PathT>
makeWORMTreeUIntGeneric(const BinaryWORMTreeUIntParams& treeParams,BufferT&& buffer);

/////////////////////
// IMPLEMENTATIONS //
/////////////////////

template <typename CursorT>
BinaryWORMTreeUIntParams findMinimumWORMTreeUIntParameters(const CursorT& c) {
  static_assert(std::is_unsigned<typename CursorT::ValueType>::value,
                "findMinimumWORMTreeUIntParameters: source tree must be of unsigned integer type");
  static_assert(sizeof(typename CursorT::ValueType) <= 8,
                "findMinimumWORMTreeUIntParameters: source tree value too large for uint64_t");

  using PathType = typename CursorT::PathType;
  BinaryWORMTreeUIntParams treeParams{};
  // do a dry run with 8 and 8 for the byte count, lttle/big endian doesn't matter
  BinaryWORMTreeUIntBuilder<std::vector<uint8_t>,PathType,false,sizeof(uint64_t),sizeof(uint64_t)> dryRunBuilder;
  auto treeIter = make_preorder_iterator<false,true>(c);
  if (!dryRunBuilder.start(true)) {
    throw std::runtime_error("Unable to start dry-run build of WORM tree!");
  }
  uint64_t maxVal = 0;
  while (!treeIter.finished()) {
    bool hasLeftChild = treeIter->canGoChildNode(0);
    bool hasRightChild = treeIter->canGoChildNode(1);
    uint64_t value{};
    bool atValue = treeIter->atValue();
    if (atValue) {
      value = *(treeIter->nodeValueRO().getPtrRO());
      if (value > maxVal) { maxVal = value; }
    }
    dryRunBuilder.addNode(treeIter->getPath(),atValue,atValue ? &value : nullptr,{hasLeftChild,hasRightChild});
    treeIter++;
  }
  if (!dryRunBuilder.finish()) {
    throw std::runtime_error("Unable to finish dry-run WORM tree!");
  }
  std::size_t valueBitsRequired = 0;
  while (maxVal > 0) { ++valueBitsRequired; maxVal = (maxVal >> 1); }
  std::size_t valueBytesRequired = (valueBitsRequired + 7)/8;
  if (valueBytesRequired == 0) { valueBytesRequired = 1; }
  treeParams.valueSize = valueBytesRequired;
  auto treeStats = dryRunBuilder.treeStats();
  treeParams.offsetSize = treeStats.minBytesForOffset();
  return treeParams;
}


template <bool LITTLEENDIAN,std::size_t OFFSETSIZE,std::size_t VALUESIZE,typename CursorT, typename BufferT>
typename std::decay<BufferT>::type
buildBinaryWORMTreeUIntBuffer(const CursorT& cursor,BufferT&& buffer) {
  using CursorType =  CursorT;
  using PathType = typename CursorType::PathType;
  using BufferType = typename std::decay<BufferT>::type;
  using BuilderType = BinaryWORMTreeUIntBuilder<BufferType,PathType,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;
  using WormValueType = typename BuilderType::ValueType;
  BuilderType wormBuilder(std::move(buffer));
  auto treeIter = make_preorder_iterator<false,true>(cursor);
  if (!wormBuilder.start(false)) {
    throw std::runtime_error("Unable to start building WORM tree!");
  }
  while (!treeIter.finished()) {
    bool hasLeftChild = treeIter->canGoChildNode(0);
    bool hasRightChild = treeIter->canGoChildNode(1);
    WormValueType wormValue{};
    bool atValue = treeIter->atValue();
    if (atValue) {
      uint64_t value = *(treeIter->nodeValueRO().getPtrRO());
      wormValue = static_cast<WormValueType>(value);
      if (static_cast<uint64_t>(wormValue) != value) {
        throw std::runtime_error("Value exceeded capacity of WORM tree value");
      }
    }
    
    wormBuilder.addNode(treeIter->getPath(),atValue,atValue ? &wormValue : nullptr,{hasLeftChild,hasRightChild});
    treeIter++;
  }
  wormBuilder.finish();
  if (!wormBuilder.start(true)) {
    throw std::runtime_error("Unable to finish building WORM tree!");
  }

  return wormBuilder.extractBuffer();
}

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE = sizeof(uint64_t),std::size_t VALUESIZE = sizeof(uint64_t)>
struct BuildBinaryWORMTreeUInt
  : public BuildBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE - 1>
{
  static_assert(OFFSETSIZE <= sizeof(uint64_t),"OFFSETSIZE > sizeof(uint64_t)");
  static_assert(VALUESIZE <= sizeof(uint64_t),"VALUESIZE > sizeof(uint64_t)");

  // basic recursion case that does the work
  using ParentClass = BuildBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE - 1>;
  template <typename BufferT>
  using TreeImpl = BinaryWORMTreeUIntGenericImpl<BufferT,PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;

  template <typename CursorType, typename BufferT>
  static BinaryWORMTreeUIntGeneric<PathT>
  from(const BinaryWORMTreeUIntParams& treeParams,const CursorType& cursor,BufferT&& buffer) {
    using BufferType = typename std::decay<BufferT>::type;
    if ((OFFSETSIZE == treeParams.offsetSize) &&
        (VALUESIZE == treeParams.valueSize) &&
        (LITTLEENDIAN == treeParams.isLittleEndian))
    {
      BufferType newBuffer = buildBinaryWORMTreeUIntBuffer<LITTLEENDIAN,OFFSETSIZE,VALUESIZE>(cursor,std::move(buffer));
      std::unique_ptr<TreeImpl<BufferType>> treeImpl{new TreeImpl<BufferType>{std::move(newBuffer)}};
      return BinaryWORMTreeUIntGeneric<PathT>{treeParams,std::move(treeImpl)};
    } else {
      return ParentClass::from(treeParams,cursor,std::move(buffer));
    }
  }
};

template <typename PathT,bool LITTLEENDIAN>
struct BuildBinaryWORMTreeUInt<PathT,LITTLEENDIAN,0,sizeof(uint64_t)> {
  // this terminates the template recursion - no need to run through 0 offset size
  template <typename BufferT>
  using TreeImpl = BinaryWORMTreeUIntGenericImpl<BufferT,PathT,LITTLEENDIAN,0,sizeof(uint64_t)>;
  template <typename CursorType, typename BufferT>
  static BinaryWORMTreeUIntGeneric<PathT>
  from(const BinaryWORMTreeUIntParams& treeParams,const CursorType&,BufferT&&) {
    throw std::runtime_error("Invalid UInt binary WORM tree params: offsetsize " + std::to_string(treeParams.offsetSize) +
                             " valuesize " + std::to_string(treeParams.valueSize));
    return BinaryWORMTreeUIntGeneric<PathT>();
  }  
};

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE>
struct BuildBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE,0>
  : public BuildBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE - 1,sizeof(uint64_t)>
{
  using ParentClass = BuildBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE - 1,sizeof(uint64_t)>;
  // this restarts the inheritance chain at OFFSETSIZE - 1
  template <typename CursorType, typename BufferType>
  static BinaryWORMTreeUIntGeneric<PathT>
  from(const BinaryWORMTreeUIntParams& treeParams,const CursorType& cursor,BufferType&& buffer) {
    return ParentClass::from(treeParams,cursor,std::move(buffer));
  }
};

template <typename CursorT,typename BufferType>
BinaryWORMTreeUIntGeneric<typename std::decay<CursorT>::type::PathType>
buildWORMTreeUIntGeneric(const BinaryWORMTreeUIntParams& treeParams,const CursorT& treeCursor)
{
  static_assert(std::is_unsigned<typename CursorT::ValueType>::value,
                "buildWORMTreeUIntBuffer: source tree must be of unsigned integer type");
  static_assert(sizeof(typename CursorT::ValueType) <= 8,
                "buildWORMTreeUIntBuffer: source tree value too large for uint64_t");
  using PathType = typename std::decay<CursorT>::type::PathType;
  BufferType buffer{};
  
  if (treeParams.isLittleEndian) {
    return BuildBinaryWORMTreeUInt<PathType,true>::from(treeParams,treeCursor,std::move(buffer));
  } else {
    return BuildBinaryWORMTreeUInt<PathType,false>::from(treeParams,treeCursor,std::move(buffer));
  }
  return BinaryWORMTreeUIntGeneric<PathType>{};
}


template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE = sizeof(uint64_t),std::size_t VALUESIZE = sizeof(uint64_t)>
struct MakeBinaryWORMTreeUInt
  : public MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE - 1>
{
  static_assert(OFFSETSIZE <= sizeof(uint64_t),"OFFSETSIZE > sizeof(uint64_t)");
  static_assert(VALUESIZE <= sizeof(uint64_t),"VALUESIZE > sizeof(uint64_t)");

  // basic recursion case that does the work
  using ParentClass = MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE - 1>;
  template <typename BufferT>
  using TreeImpl = BinaryWORMTreeUIntGenericImpl<BufferT,PathT,LITTLEENDIAN,OFFSETSIZE,VALUESIZE>;

  template <typename BufferT>
  static BinaryWORMTreeUIntGeneric<PathT>
  from(const BinaryWORMTreeUIntParams& treeParams,BufferT&& buffer) {
    using BufferType = typename std::decay<BufferT>::type;
    if ((OFFSETSIZE == treeParams.offsetSize) &&
        (VALUESIZE == treeParams.valueSize) &&
        (LITTLEENDIAN == treeParams.isLittleEndian))
    {
      std::unique_ptr<TreeImpl<BufferType>> treeImpl{new TreeImpl<BufferType>{std::move(buffer)}};
      return BinaryWORMTreeUIntGeneric<PathT>{treeParams,std::move(treeImpl)};
    } else {
      return ParentClass::from(treeParams,std::move(buffer));
    }
  }
};

template <typename PathT,bool LITTLEENDIAN>
struct MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,0,sizeof(uint64_t)> {
  // this terminates the template recursion - no need to run through 0 offset size
  template <typename BufferT>
  using TreeImpl = BinaryWORMTreeUIntGenericImpl<BufferT,PathT,LITTLEENDIAN,0,sizeof(uint64_t)>;
  template <typename CursorType, typename BufferT>
  static BinaryWORMTreeUIntGeneric<PathT>
  from(const BinaryWORMTreeUIntParams& treeParams,BufferT&&) {
    throw std::runtime_error("Invalid UInt binary WORM tree params: offsetsize " + std::to_string(treeParams.offsetSize) +
                             " valuesize " + std::to_string(treeParams.valueSize));
    return BinaryWORMTreeUIntGeneric<PathT>();
  }
};

template <typename PathT,bool LITTLEENDIAN,std::size_t OFFSETSIZE>
struct MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE,0>
  : public MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE - 1,sizeof(uint64_t)>
{
  using ParentClass = MakeBinaryWORMTreeUInt<PathT,LITTLEENDIAN,OFFSETSIZE - 1,sizeof(uint64_t)>;
  // this restarts the inheritance chain at OFFSETSIZE - 1
  template <typename CursorType, typename BufferType>
  static BinaryWORMTreeUIntGeneric<PathT>
  from(const BinaryWORMTreeUIntParams& treeParams,BufferType&& buffer) {
    return ParentClass::from(treeParams,std::move(buffer));
  }
};

template <typename PathT,typename BufferT>
BinaryWORMTreeUIntGeneric<PathT>
makeWORMTreeUIntGeneric(const BinaryWORMTreeUIntParams& treeParams,BufferT&& buffer) {
  static_assert(std::is_rvalue_reference<BufferT>::value,"makeWORMTreeUIntGeneric: need rvalue reference to buffer");
  if (treeParams.isLittleEndian) {
    return MakeBinaryWORMTreeUInt<PathT,true>::from(treeParams,std::move(buffer));
  } else {
    return MakeBinaryWORMTreeUInt<PathT,false>::from(treeParams,std::move(buffer));
  }
}

} // namespace RadixTree
} // namespace Mapper
} // namespace Akamai

#endif