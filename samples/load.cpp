// Copyright (C) 2021-2022 Zukaritasu
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


#include "Properties.hpp"

#include <iostream>

int main()
{
	Properties prop;

	/* In the user.properties file ->
		
		Name=Zukaritasu
		Country=Venezuela
	*/
	if (!prop.Load("user.properties")) {
		// Read error
		std::cout << "Error reading user information\n";
		return 1;
	}
	
	std::cout << prop.GetValue("Name") << std::endl;
	/* Console output ->
		
		Zukaritasu
	*/

	return 0;
}