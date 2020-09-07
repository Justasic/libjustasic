/** Class with the ability to keep flags on items, they should extend from this
 * where T is an enum.
 */
template<typename T, size_t Size = 32> class Flags
{
 protected:
	std::bitset<Size> Flag_Values;

 public:
	/** Add a flag to this item
	 * @param Value The flag
	 */
	void SetFlag(T Value)
	{
		Flag_Values[Value] = true;
	}

	/** Remove a flag from this item
	 * @param Value The flag
	 */
	void UnsetFlag(T Value)
	{
		Flag_Values[Value] = false;
	}

	/** Check if this item has a flag
	 * @param Value The flag
	 * @return true or false
	 */
	bool HasFlag(T Value) const
	{
		return Flag_Values.test(Value);
	}

	/** Check how many flags are set
	 * @return The number of flags set
	 */
	size_t FlagCount() const
	{
		return Flag_Values.count();
	}

	/** Unset all of the flags
	 */
	void ClearFlags()
	{
		Flag_Values.reset();
	}
};
