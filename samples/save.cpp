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
	prop.Add("Name", "Zukaritasu");
	prop.Add("Country", "Venezuela");
	if (!prop.Save("user.properties")) {
		/* Read error */
		std::cout << "Error saving information\n";
		return 1;
	}
	
	std::cout << "Success saving the information!\n";
	
	/* In the file ->
		
		Name=Zukaritasu
		Country=Venezuela
	*/
	return 0;
}	