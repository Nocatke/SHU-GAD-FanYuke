/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2025 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include <map>
#include <string>
#include <vector>
#include <utility>

namespace AK
{
	namespace WwiseAuthoringAPI
	{
		template<typename VariantType, typename StringType, typename StringCompareType> class AkJsonBase final
		{
		public:

			typedef std::map<StringType, AkJsonBase, StringCompareType> Map;
			typedef std::vector<AkJsonBase> Array;

			enum class Type
			{
				Map,
				Array,
				Variant,
				Empty
			};

			AkJsonBase() {}

			AkJsonBase(const VariantType& in_other)
			{
				SetVariant(in_other);
			}

			AkJsonBase(const Array& in_other)
			{
				SetArray(in_other);
			}

			AkJsonBase(const Map& in_other)
			{
				SetMap(in_other);
			}

			AkJsonBase(Type in_eType)
			{
				SetType(in_eType);
			}

			AkJsonBase(const AkJsonBase& in_other)
			{
				Copy(in_other, *this);
			}

			AkJsonBase(AkJsonBase&& in_other) noexcept
			{
				m_ptr = in_other.m_ptr;
				m_eType = in_other.m_eType;

				in_other.m_ptr = nullptr;
				in_other.m_eType = Type::Empty;
			}

			~AkJsonBase()
			{
				Clear();
			}

			inline bool IsEmpty() const
			{
				return (m_eType == Type::Empty);
			}

			inline AkJsonBase& operator=(const AkJsonBase& in_other)
			{
				Copy(in_other, *this);
				return *this;
			}

			inline AkJsonBase& operator=(AkJsonBase&& in_other) noexcept
			{
				std::swap(this->m_eType, in_other.m_eType);
				std::swap(this->m_ptr, in_other.m_ptr);
				return *this;
			}

			void SetVariant(const VariantType& in_other)
			{
				SetType(Type::Variant);
				*m_pVariant = in_other;
				AKASSERT(*m_pVariant == in_other);
				AKASSERT(m_pVariant->GetType() == in_other.GetType());
			}

			void SetArray(const Array& in_other)
			{
			#ifdef _DEBUG
				for (const auto& element : in_other)
				{
					AKASSERT(!element.IsEmpty());
				}
			#endif
				SetType(Type::Array);
				(*m_pArray).assign(in_other.begin(), in_other.end());
				AKASSERT(m_pArray->size() == in_other.size());
			}

			void SetMap(const Map& in_other)
			{
			#ifdef _DEBUG
				for (const auto& element : in_other)
				{
					AKASSERT(!element.second.IsEmpty());
				}
			#endif
				SetType(Type::Map);
				(*m_pMap).clear();
				(*m_pMap).insert(in_other.begin(), in_other.end());
				AKASSERT(m_pMap->size() == in_other.size());
			}

			static void Copy(const AkJsonBase& in_rSrc, AkJsonBase& in_rDest)
			{
				switch (in_rSrc.GetType())
				{
				case Type::Array:
				{
					in_rDest.SetArray(in_rSrc.GetArray());
					AKASSERT(in_rDest.GetType() == Type::Array);
					break;
				}
				case Type::Map:
				{
					in_rDest.SetMap(in_rSrc.GetMap());
					AKASSERT(in_rDest.GetType() == Type::Map);
					break;
				}
				case Type::Variant:
				{
					in_rDest.SetVariant(in_rSrc.GetVariant());
					AKASSERT(in_rDest.GetType() == Type::Variant);
					break;
				}
				case Type::Empty:
				{
					in_rDest.Clear();
					AKASSERT(in_rDest.GetType() == Type::Empty);
					break;
				}
				default:
				{
					AKASSERT(!"Invalid type copied");
					in_rDest.Clear();
					AKASSERT(in_rDest.GetType() == Type::Empty);
					break;
				}
				}
			}

			void SetType(Type in_eType)
			{
				if (in_eType != m_eType)
				{
					Clear();
					AKASSERT(m_ptr == nullptr);

					m_eType = in_eType;
					switch (in_eType)
					{
					case Type::Array:
						m_pArray = new Array();
						break;
					case Type::Map:
						m_pMap = new Map();
						break;
					case Type::Variant:
						m_pVariant = new VariantType();
						break;
					case Type::Empty:
						m_eType = Type::Empty;
						AKASSERT(!L"Cannot assign type empty");
						break;
					default:
						m_eType = Type::Empty;
						AKASSERT(!L"Unknown type assigned");
						break;
					}
				}
				AKASSERT(m_eType == in_eType);
				AKASSERT(m_ptr != nullptr);
			}

			void Clear()
			{
				if (m_ptr == nullptr)
				{
					AKASSERT(m_eType == Type::Empty);
					return;
				}

				switch (m_eType)
				{
				case Type::Array:
					delete m_pArray;
					break;
				case Type::Map:
					delete m_pMap;
					break;
				case Type::Variant:
					delete m_pVariant;
					break;
				case Type::Empty:
				default:
					// Leak any value pointed to by m_ptr: unknown type
					AKASSERT(!"Empty-typed variant with non-null value");
					break;
				}

				m_eType = Type::Empty;
				m_ptr = nullptr;
			}

			inline Type GetType() const
			{
				AKASSERT(
					m_eType == Type::Map ||
					m_eType == Type::Array ||
					m_eType == Type::Variant ||
					m_eType == Type::Empty
				);
				return m_eType;
			}

			inline bool IsArray() const
			{
				return m_eType == Type::Array;
			}

			inline bool IsMap() const
			{
				return m_eType == Type::Map;
			}

			inline bool IsVariant() const
			{
				return m_eType == Type::Variant;
			}

			inline Array& GetArray() const
			{
				AKASSERT(m_eType == Type::Array);
				AKASSERT(m_pArray != nullptr);
				return *m_pArray;
			}

			inline const Map& GetMap() const
			{
				AKASSERT(m_eType == Type::Map);
				AKASSERT(m_pMap != nullptr);
				return *m_pMap;
			}

			inline const VariantType& GetVariant() const
			{
				AKASSERT(m_eType == Type::Variant);
				AKASSERT(m_pVariant != nullptr);
				return *m_pVariant;
			}

			inline Array& GetArray()
			{
				AKASSERT(m_eType == Type::Array);
				AKASSERT(m_pArray != nullptr);
				return *m_pArray;
			}

			inline Map& GetMap()
			{
				AKASSERT(m_eType == Type::Map);
				AKASSERT(m_pMap != nullptr);
				return *m_pMap;
			}

			inline VariantType& GetVariant()
			{
				AKASSERT(m_eType == Type::Variant);
				AKASSERT(m_pVariant != nullptr);
				return *m_pVariant;
			}

			inline bool HasKey(const StringType& in_key) const
			{
				if (m_eType == Type::Map)
				{
					return (GetMap().find(in_key) != GetMap().end());
				}
				else
				{
					AKASSERT(!"Calling HasKey on AkJsonBase which is NOT a map!");
				}

				return false;
			}

			const AkJsonBase& operator[](const StringType& in_key) const
			{
				if (m_eType == Type::Map)
				{
					auto it = GetMap().find(in_key);
					AKASSERT(it != GetMap().end());
					if (it != GetMap().end())
						return it->second;
				}
				else
				{
					AKASSERT(!"Calling [] operator on AkJsonBase which is NOT a map!");
				}
				static const AkJsonBase empty;
				return empty;
			}

			const AkJsonBase& operator[](const uint32_t in_index) const
			{
				if (m_eType == Type::Array)
				{
					AKASSERT(in_index < GetArray().size());
					if (in_index < GetArray().size())
						return GetArray()[in_index];
				}
				else
				{
					AKASSERT(!"Calling [] operator on AkJsonBase which is NOT an array!");
				}
				static const AkJsonBase empty;
				return empty;
			}

			// Implicit interface compatible with rapidjson. Dependency on rapidjson is not required if those functions are not used.
			template<typename RapidJsonValue, typename RapidJsonAllocator, typename RapidJsonSizeType, typename StringToValue>
			static bool ToRapidJson(const AkJsonBase& in_node, RapidJsonValue& out_rapidJson, RapidJsonAllocator& in_allocator);

			template<typename RapidJsonValue>
			static bool FromRapidJson(const RapidJsonValue& in_rapidJson, AkJsonBase& out_node);

		private:

			Type m_eType = Type::Empty;
			union
			{
				void* m_ptr{nullptr};
				Map* m_pMap;
				Array* m_pArray;
				VariantType* m_pVariant;
			};
		};

		//<rapidjson::Value>
		template<typename VariantType, typename StringType, typename StringCompareType>
		template<typename RapidJsonValue>
		bool AkJsonBase<VariantType, StringType, StringCompareType>::FromRapidJson(const RapidJsonValue& in_rapidJson, AkJsonBase& out_node)
		{
			if (in_rapidJson.IsObject())
			{
				out_node.SetType(AkJsonBase::Type::Map);
				AkJsonBase::Map& map = out_node.GetMap();

				for (auto it = in_rapidJson.MemberBegin(); it != in_rapidJson.MemberEnd(); ++it)
				{
					StringType rosName(it->name.GetString());
					map[rosName] = AkJsonBase();
					if (!FromRapidJson(it->value, map[rosName]))
						return false;
				}
			}
			else if (in_rapidJson.IsArray())
			{
				out_node.SetType(AkJsonBase::Type::Array);
				AkJsonBase::Array& array = out_node.GetArray();

				for (auto it = in_rapidJson.Begin(); it != in_rapidJson.End(); ++it)
				{
					array.push_back(AkJsonBase());
					if (!FromRapidJson(*it, array.back()))
						return false;
				}
			}
			else if (in_rapidJson.IsNull())
			{
				out_node = VariantType();
				return true;
			}
			else
			{
				out_node.SetType(AkJsonBase::Type::Variant);
				out_node.SetVariant(VariantType::template FromRapidJsonValue<decltype(in_rapidJson)>(in_rapidJson));

				return (!out_node.GetVariant().IsEmpty());
			}

			return true;
		}

		//<rapidjson::Value, rapidjson::MemoryPoolAllocator<>, rapidjson::SizeType>
		template<typename VariantType, typename StringType, typename StringCompareType>
		template<typename RapidJsonValue, typename RapidJsonAllocator, typename RapidJsonSizeType, typename StringToValue>
		bool AkJsonBase<VariantType, StringType, StringCompareType>::ToRapidJson(const AkJsonBase& in_node, RapidJsonValue& out_rapidJson, RapidJsonAllocator& in_allocator)
		{
			if (in_node.GetType() == AkJsonBase::Type::Map)
			{
				out_rapidJson.SetObject();

				const AkJsonBase::Map& map = in_node.GetMap();
				for (auto it = map.begin(); it != map.end(); ++it)
				{
					RapidJsonValue value;
					if (!ToRapidJson<RapidJsonValue, RapidJsonAllocator, RapidJsonSizeType, StringToValue>(it->second, value, in_allocator))
						return false;

					StringType name(it->first);
					RapidJsonValue key = StringToValue::Convert(name, in_allocator);

					out_rapidJson.AddMember(key, value, in_allocator);
				}
			}
			else if (in_node.GetType() == AkJsonBase::Type::Array)
			{
				out_rapidJson.SetArray();

				const AkJsonBase::Array& arr = in_node.GetArray();
				for (auto it = arr.begin(); it != arr.end(); ++it)
				{
					RapidJsonValue value;
					if (!ToRapidJson<RapidJsonValue, RapidJsonAllocator, RapidJsonSizeType, StringToValue>(*it, value, in_allocator))
						return false;

					out_rapidJson.PushBack(value, in_allocator);
				}
			}
			else if (in_node.GetType() == AkJsonBase::Type::Empty)
			{
				out_rapidJson.SetNull();
			}
			else
			{
				const VariantType& value = in_node.GetVariant();
				return value.template toRapidJsonValue<decltype(out_rapidJson), decltype(in_allocator), RapidJsonSizeType>(out_rapidJson, in_allocator);
			}

			return true;
		}
	}
}
