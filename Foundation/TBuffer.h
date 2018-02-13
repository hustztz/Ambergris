#pragma once

namespace ambergris {

	/// <description>
	/// A generic buffer class, can be used as dynamic array or stack.
	/// </description>
	template<class T>
	class TBuffer
	{
	public:
		/// <description>
		/// Constructor.
		/// </description>
		TBuffer() :mpData(nullptr), mSize(0), mCapacity(0) {}

		/// <description>
		/// Constructor with given capacity.
		/// </description>
		TBuffer(unsigned int capacity) :mSize(0), mCapacity(capacity)
		{
			mpData = new T[capacity];
		}

		/// <description>
		/// Constructor with given capacity.
		/// </description>
		TBuffer(const TBuffer& buff) :mpData(nullptr), mSize(0), mCapacity(0)
		{
			*this = buff;
		}

		/// <description>
		/// Move constructor.
		/// </description>
		TBuffer(TBuffer&& buffer) :mpData(nullptr), mSize(0), mCapacity(0)
		{
			mpData = buffer.mpData;
			mSize = buffer.mSize;
			mCapacity = buffer.mCapacity;

			buffer.mpData = NULL;
			buffer.mSize = 0;
			buffer.mCapacity = 0;
		}

		/// <description>
		/// Destructor.
		/// </description>
		~TBuffer()
		{
			Cleanup();
		}


		/// <description>
		/// Get the cached data, its readonly.
		/// </description>
		const T* GetData() const
		{
			return mpData;
		}


		/// <description>
		/// Get the cached data size, it's object count.
		/// </description>
		unsigned int GetSize() const
		{
			return mSize;
		}

		/// <description>
		/// Returns true if the RCBuffer is empty.
		/// </description>
		bool Empty() const
		{
			return mSize == 0;
		}

		/// <description>
		/// Get the buffer's capacity, it's object count.
		/// </description>
		unsigned int GetCapacity() const
		{
			return mCapacity;
		}

		/// <description>
		/// Append a data object to the buffer.
		/// </description>
		void Append(const T& data)
		{
			// if reach maximum size
			if (mSize == mCapacity)
			{
				Reserve(mCapacity == 0 ? 1 : mCapacity * 2);
			}

			// copy data.
			mpData[mSize++] = data;
		}

		/// <description>
		/// Remove an object from the buffer. This does not preserve the order of the list.
		/// </description>
		void RemoveFast(unsigned int index)
		{
			_RemoveFast(index);
		}

		/// <description>
		/// Remove an object from the buffer. This does not preserve the order of the list.
		/// </description>
		void RemoveFast(const T& t)
		{
			_RemoveFast(FindIndex(t));
		}

		/// <description>
		/// Remove an object from the buffer. This does preserve the order of the list.
		/// </description>
		void RemoveStable(unsigned int index)
		{
			_RemoveStable(index);
		}


		/// <description>
		/// Remove an object from the buffer. This does preserve the order of the list.
		/// </description>
		void RemoveStable(const T& t)
		{
			_RemoveStable(FindIndex(t));
		}

		/// <description>
		/// Finds the index of the given object, or returns the same as GetSize().
		/// </description>
		unsigned int FindIndex(const T& t)
		{
			for (unsigned int i = 0; i != mSize; ++i)
			{
				if (mpData[i] == t) return i;
			}
			return mSize;
		}


		/// <description>
		/// Get an element of this buffer at a given index.
		/// </description>
		const T* operator[](unsigned int index) const
		{
			if (index < GetSize())
			{
				return &mpData[index];
			}
			else return 0;
		}

		/// <description>
		/// Get an element of this buffer at a given index.
		/// </description>
		T* operator[](unsigned int index)
		{
			if (index < GetSize())
			{
				return &mpData[index];
			}
			else return 0;
		}

		/// <description>
		/// Append a group of data object to the buffer.
		/// </description>
		void Append(const T* data, unsigned int size)
		{
			// check if need to resize the buffer.
			if (mSize + size > mCapacity)
			{
				Reserve((mSize + size) * 2);
			}

			// copy data.
			for (unsigned int i = 0; i < size; ++i)
				mpData[mSize + i] = data[i];
			mSize += size;
		}

		/// <description>
		/// Get the last object of the buffer for read.
		/// </description>
		const T* GetTail() const
		{
			return GetTailHelper();
		}

		/// <description>
		/// Get the last object of the buffer for read and write.
		/// </description>
		T* GetTail()
		{
			return const_cast<T*>(GetTailHelper());
		}

		/// <description>
		/// Reserve current buffer size.
		/// </description>
		void Reserve(unsigned int capacity)
		{
			if (capacity > mCapacity)
			{

				T *pNewData = new T[capacity];
				for (unsigned int i = 0; i < mSize; ++i)
					pNewData[i] = mpData[i];

				delete[] mpData;
				mpData = pNewData;

				mCapacity = capacity;
			}
		}

		/// <description>
		/// Resize the current buffer to the given size.
		/// </description>
		void Resize(unsigned int size)
		{
			if (mSize < size)
			{
				Reserve(size);
			}
			mSize = size;
		}

		/// <description>
		/// Resize the current buffer to the given size.
		/// </description>
		void Resize(unsigned int size, const T& defaultT)
		{
			unsigned int oldSize = mSize;
			Resize(size);
			for (unsigned int i = oldSize; i < mSize; ++i)
			{
				mpData[i] = defaultT;
			}
		}

		/// <description>
		/// Reset the buffer, only change size and doesn't deallocate buffer.
		/// </description>
		void Reset()
		{
			mSize = 0;
		}

		/// <description>
		/// clean all data in the buffer.
		/// </description>
		void Cleanup()
		{
			if (mpData != 0)
			{
				delete[] mpData;
				mpData = 0;
			}

			mCapacity = 0;
			mSize = 0;
		}

		bool operator==(const TBuffer& buff) const
		{
			unsigned int size = GetSize();
			if (buff.GetSize() != size)
				return false;

			for (unsigned int i = 0; i < size; ++i)
				if (!(mpData[i] == buff.mpData[i]))
					return false;

			return true;
		}

		TBuffer& operator=(const TBuffer& buff)
		{
			unsigned int size = buff.GetSize();
			Reserve(size);
			mSize = size;

			for (unsigned int i = 0; i < size; ++i)
				mpData[i] = buff.mpData[i];

			return *this;
		}

		/// <description>
		/// Move assignment operator.
		/// </description>
		TBuffer& operator=(TBuffer&& buffer)
		{
			if (this != &buffer)
			{
				Cleanup();

				mpData = buffer.mpData;
				mSize = buffer.mSize;
				mCapacity = buffer.mCapacity;

				buffer.mpData = NULL;
				buffer.mSize = 0;
				buffer.mCapacity = 0;
			}
			return *this;
		}

		void Swap(TBuffer& buff)
		{
			{ T*           temp = mpData;     mpData = buff.mpData;     buff.mpData = temp; }
			{ unsigned int temp = mSize;      mSize = buff.mSize;      buff.mSize = temp; }
			{ unsigned int temp = mCapacity;  mCapacity = buff.mCapacity;  buff.mCapacity = temp; }
		}


	private:
		/// <description>
		/// Helper function to get last member.
		/// </description>
		const T* GetTailHelper() const
		{
			if ((mpData == 0) || (mSize == 0))
				return 0;
			return &mpData[mSize - 1];
		}

		inline void _RemoveFast(unsigned int index)
		{
			if (index < mSize)
			{
				mpData[index] = mpData[mSize - 1];
				mSize--;
			}
		}

		inline void _RemoveStable(unsigned int index)
		{
			if (index < mSize)
			{
				mSize--;

				// if the index is not the last element, copy the last part of the buffer down
				if (index < mSize)
				{
					for (unsigned int i = 0; i < mSize - index; ++i)
						mpData[index + i] = mpData[index + i + 1];
				}
			}
		}

	protected:
		T*             mpData;
		unsigned int   mSize;
		unsigned int   mCapacity;

	};
}