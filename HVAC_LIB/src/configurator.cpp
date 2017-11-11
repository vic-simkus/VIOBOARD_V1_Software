/*
* This file is part of the software stack for Vic's IO board and its
* associated projects.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* Copyright 2016,2017,2018 Vidas Simkus (vic.simkus@gmail.com)
*/


#include "lib/configurator.hpp"
#include "lib/exceptions.hpp"
#include "lib/config.hpp"
#include "lib/string_lib.hpp"

#include <unistd.h>		// getcwd
#include <stdlib.h>		// realpath, access
#include <string.h>		// memset
#include <limits.h> 	// PATH_MAX

#include <sys/types.h>	// open and various flags
#include <sys/stat.h>	//	"		"		"
#include <fcntl.h>		//	"		"		"

#include <stdexcept>	// exceptions

#include <stdio.h>		// fopen, fgetc

#include <vector>		// vector

using namespace BBB_HVAC;
using namespace std;

CONFIGURATOR::CONFIGURATOR(const string& _file)
{
	INIT_LOGGER("BBB_HVAC::CONFIGURATOR");
	this->file_name = _file;
	this->buffer = (char*) malloc(GC_BUFFER_SIZE);
	memset(this->buffer, 0, GC_BUFFER_SIZE);
	return;
}
CONFIGURATOR::~CONFIGURATOR()
{
	memset(this->buffer, 0, GC_BUFFER_SIZE);
	free(this->buffer);
	this->buffer = nullptr;
	this->config_entries.clear();
	return;
}

void CONFIGURATOR::normalize_file_names(void) throw(exception)
{
	char* _buff = (char*) malloc(PATH_MAX);
	memset(_buff, 0, PATH_MAX);
	getcwd(_buff, PATH_MAX);
	this->cwd = string(_buff);
	memset(_buff, 0, PATH_MAX);
	realpath(this->file_name.data(), _buff);
	this->file_name = string(_buff);
	this->overlay_file_name = this->file_name + ".overlay";
	memset(_buff, 0, PATH_MAX);
	free(_buff);
	return;
}

void CONFIGURATOR::check_file_permissions(void) throw(exception)
{
	if(access(this->file_name.data(), F_OK) != 0)
	{
		throw runtime_error("Configuration file [" + this->file_name + "] does not exist.");
	}

	if(access(this->file_name.data(), R_OK) != 0)
	{
		throw runtime_error("Configuration file [" + this->file_name + "] is not readable to us.");
	}

	if(access(this->overlay_file_name.data(), F_OK) != 0)
	{
		LOG_DEBUG(this->overlay_file_name + " does not exist.");
		int fd = open(this->overlay_file_name.data(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

		if(fd == -1)
		{
			throw runtime_error(create_perror_string("Failed to create overlay file"));
		}

		close(fd);
	}

	return;
}

void CONFIGURATOR::process_file(void) throw(exception)
{
	memset(buffer, 0, GC_BUFFER_SIZE);
	int c = 0;
	size_t buff_idx = 0;
	size_t line_idx = 0;
	char cin = 0;
	FILE* file = fopen(this->file_name.data(), "rt");

	while((c = fgetc(file)) != EOF)
	{
		cin = (char) c;

		if(cin == '\n')
		{
			/*
			 * End of the line
			 */
			this->buffer[buff_idx] = '\0';
			/*
			 * Process line here.
			 */
			//LOG_DEBUG("End of line [" + num_to_str(line_idx) + "] @ " + num_to_str(buff_idx));
			this->process_line(line_idx);
			memset(this->buffer, 0, buff_idx);
			buff_idx = 0;
			line_idx += 1;
		}
		else
		{
			this->buffer[buff_idx] = (char) cin;
			buff_idx += 1;
		}

		if(buff_idx == GC_BUFFER_SIZE)
		{
			fclose(file);
			throw runtime_error("Failed to find line terminator before exceeding buffer size.");
		}
	}

	//LOG_DEBUG("Lines processed: " + num_to_str(line_idx));
	fclose(file);
	return;
}
void CONFIGURATOR::process_line(size_t _line_idx) throw(exception)
{
	char* line = trim_string(this->buffer);
	size_t line_length = strlen(line);
	size_t prev_tab = 0;

	//LOG_DEBUG("Processing line: " + num_to_str(_line_idx) + ".  Length: " + num_to_str(line_length));

	if(line_length == 0 || line[0] == '#')
	{
		return;
	}

	char c;
	vector<string> line_parts;

	for(size_t i = 0; i < line_length; i++)
	{
		c = line[i];

		if(c == '\t')
		{
			line[i] = '\0';

			if(strlen((line + prev_tab)) > 0)
			{
				line_parts.push_back((line + prev_tab));
			}
			else
			{
				LOG_WARNING("A zero-length part was found on line " + num_to_str(_line_idx) + " around field " + num_to_str(line_parts.size()) + ".  Possible double-tab.");
			}

			prev_tab = i + 1;
		}
	}

	line_parts.push_back(line + prev_tab);
	ENUM_CONFIG_TYPES type;

	try
	{
		type = CONFIG_ENTRY::string_to_type(line_parts[0]);
	}
	catch(const exception& _e)
	{
		LOG_ERROR(string("Failed to process configuration line " + num_to_str(_line_idx) + ": " + _e.what()));
		return;
	}

	if(type == ENUM_CONFIG_TYPES::INVALID)
	{
		LOG_ERROR(string("An INVALID record type specified in the configuration file at line " + num_to_str(_line_idx) + "?"));
		return;
	}

	line_parts.erase(line_parts.begin());
	size_t ces = this->config_entries.size();
	this->config_entries.push_back(CONFIG_ENTRY(type, line_parts));

	switch(type)
	{
		case ENUM_CONFIG_TYPES::DI:
			this->di_configs.push_back(ces);
			break;

		case ENUM_CONFIG_TYPES::DO:
			this->do_configs.push_back(ces);
			break;

		case ENUM_CONFIG_TYPES::AI:
			this->ai_configs.push_back(ces);
			break;

		case ENUM_CONFIG_TYPES::AO:
			this->ao_configs.push_back(ces);
			break;

		case ENUM_CONFIG_TYPES::SP:
			this->sp_configs.push_back(ces);
			break;

		case ENUM_CONFIG_TYPES::BOARD:
			this->board_configs.push_back(ces);
			break;

		case ENUM_CONFIG_TYPES::INVALID:
			//this is caought above.  This is here for the compiler warning.
			break;
	}

	return;
}
void CONFIGURATOR::read_file(void) throw(exception)
{
	this->normalize_file_names();
	LOG_DEBUG("Configuration file: " + this->file_name);
	LOG_DEBUG("Overlay file: " + this->overlay_file_name);
	this->check_file_permissions();
	this->process_file();
	return;
}
void CONFIGURATOR::write_file(void) const throw(exception)
{
	FILE* out_file = fopen(this->overlay_file_name.data(), "wt");
	fprintf(out_file, "#\n");
	fprintf(out_file, "# This file is mechanically generated.  Manual edits will most likely be lost.  So don't do it.\n");
	fprintf(out_file, "#\n");

	for(CONFIG_ENTRY_LIST_TYPE::const_iterator i = this->config_entries.cbegin(); i != this->config_entries.cend(); ++i)
	{
		if(i->get_is_dirty())
		{
			fprintf(out_file, "%s\n", i->write_self_to_file().data());
		}
	}

	fprintf(out_file, "# EOF\n");
	fclose(out_file);
	return;
}

const CONFIG_TYPE_INDEX_TYPE& CONFIGURATOR::get_do_index(void) const
{
	return this->do_configs;
}
const CONFIG_TYPE_INDEX_TYPE& CONFIGURATOR::get_ai_index(void) const
{
	return this->ai_configs;
}
const CONFIG_TYPE_INDEX_TYPE& CONFIGURATOR::get_sp_index(void) const
{
	return this->sp_configs;
}
const CONFIG_TYPE_INDEX_TYPE& CONFIGURATOR::get_board_index(void) const
{
	return this->board_configs;
}

CONFIG_ENTRY& CONFIGURATOR::get_config_entry(size_t _idx) throw(exception)
{
	if(_idx > this->config_entries.size())
	{
		throw runtime_error(string("Supplied index " + num_to_str(_idx) + " is out of range."));
	}

	return this->config_entries[_idx];
}

const CONFIG_ENTRY& CONFIGURATOR::get_type_by_id(ENUM_CONFIG_TYPES _type, const string& _id) throw(exception)
{
	const CONFIG_TYPE_INDEX_TYPE* idx;

	switch(_type)
	{
		case ENUM_CONFIG_TYPES::AI:
			idx = &(this->ai_configs);
			break;

		case ENUM_CONFIG_TYPES::AO:
			idx = &(this->ao_configs);
			break;

		case ENUM_CONFIG_TYPES::DI:
			idx = &(this->di_configs);
			break;

		case ENUM_CONFIG_TYPES::DO:
			idx = &(this->do_configs);
			break;

		case ENUM_CONFIG_TYPES::SP:
			idx = &(this->sp_configs);
			break;

		case ENUM_CONFIG_TYPES::INVALID:
			THROW_EXCEPTION(runtime_error, "INVALID configuration record type passed.  Why?");
			break;

		default:
			THROW_EXCEPTION(runtime_error, "Unexpected code path.");
			break;
	}

	for(CONFIG_TYPE_INDEX_TYPE::const_iterator i = idx->cbegin(); i != idx->cend(); ++i)
	{
		const CONFIG_ENTRY& e = this->get_config_entry(*i);

		if(e.get_id() == _id)
		{
			return e;
		}
	}

	THROW_EXCEPTION(range_error,"Failed to find item");
}
