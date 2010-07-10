#ifndef _K_ALGORITHMS_H
#define _K_ALGORITHMS_H

/** @namespace k_std
 * @brief The kernel standard template library namespace.
 */
namespace k_std
{

///Searches [first, last) for the item value.
template<typename T, typename Iterator>
inline Iterator find(Iterator first, Iterator last, const T &value)
{
	Iterator	it = first;

	for( ; it != last; ++it)
		if(*it == value)
			break;

	return(it);
}

///Searches (last, first] for the item value.
template<typename T, typename Iterator>
inline Iterator find_backwards(Iterator first, Iterator last, const T &value)
{
	Iterator	it = last;

	for(--it; it != first; --it)
		if(*it == value)
			return(it);

	if(*it == value)
		return(it);

	return(last);
}

///Searches [first, last) for when Comparitor(*it) is true and returns first instance that is true.
template<typename Comparitor, typename Iterator>
inline Iterator find_if(Iterator first, Iterator last, Comparitor cmp)
{
	Iterator it = first;

	for( ; it != last; ++it)
		if(cmp(*it))
			break;
	
	return(it);
}

///Searches (last, first] for when Comparitor(*it) is true.
template<typename Comparitor, typename Iterator>
inline Iterator find_if_backwards(Iterator first, Iterator last, Comparitor cmp)
{
	Iterator	it = last;

	for(--it; it != first; --it)
		if(cmp(*it))
			return(it);

	if(cmp(*it))
		return(it);

	return(last);
}

}

#endif
