{$mode fpc}
unit mcfg;

{ Bindings to MCFG/2 0.5.0 for freepascal }

interface

{$packrecords c}

type
	TMcfgFieldType = (TYPE_INVALID = -1,
					  TYPE_STRING,
					  TYPE_LIST,
					  TYPE_BOOL,
					  TYPE_I8,
					  TYPE_U8,
					  TYPE_I16,
					  TYPE_U16,
					  TYPE_I32,
					  TYPE_U32);

	TMcfgField = record
		name: PChar;
		_type: TMcfgFieldType;
		data: Pointer;
		size: SizeUInt;
	end;

	PMcfgField = ^TMcfgField;

	TMcfgList = record
		_type: TMcfgFieldType;
		field_count: SizeUInt;
		fields: PMcfgField;
	end;

	PMcfgList = ^TMcfgList;

	TMcfgSection = record
		name: PChar;
		field_count: SizeUInt;
		fields: PMcfgField;
	end;

	PMcfgSection = ^TMcfgSection;

	TMcfgSector = record
		name: PChar;
		section_count: SizeUInt;
		sections: PMcfgSection;
	end;

	PMcfgSector = ^TMcfgSector;

	TMcfgFile = record
		sector_count:SizeUInt;
		sectors:PMcfgSector;

		dynfield_count:SizeUInt;
		dynfields:PMcfgField;
	end;

	PMcfgFile = ^TMcfgFile;

	TMcfgErr = (MCFG_OK,
				MCFG_TODO,
				MCFG_INVALID_PARSER_STATE,
				MCFG_SYNTAX_ERROR,
				MCFG_INVALID_KEYWORD,
				MCFG_END_IN_NOWHERE,
				MCFG_STRUCTURE_ERROR,
				MCFG_DUPLICATE_SECTOR,
				MCFG_DUPLICATE_SECTION,
				MCFG_DUPLICATE_FIELD,
				MCFG_DUPLICATE_DYNFIELD,
				MCFG_INVALID_TYPE,
				MCFG_NULLPTR,
				MCFG_INTEGER_OUT_OF_BOUNDS,
				MCFG_MALLOC_FAIL,
				MCFG_OS_ERROR_MASK = $f000);

	TMcfgLinespan = record
		starting_line: SizeUInt;

		line_count: SizeUInt;
	end;

	TMcfgParseResult = record
		err: TMcfgErr;

		err_linespan: TMcfgLinespan;

		value: TMcfgFile;
	end;

	TMcfgString = record
		capacity: UInt64;

		length: UInt64;

		{ imaginary char data[]; }
	end;

	PMcfgString = ^TMcfgString;

	TMcfgSerializeResult = record
		err: TMcfgErr;

		value: PMcfgString;
	end;

	TMcfgSerializeOptions = record
		tab_indentation: Boolean;

		space_count: Integer;
	end;

{$packrecords default}

function mcfg_err_string(err: TMcfgErr): PChar; cdecl; external;

{ general api }

function mcfg_sizeof(_type: TMcfgFieldType): SizeInt; cdecl; external;

function mcfg_add_sector(_file: PMcfgFile; name: PChar): TMcfgErr; cdecl; external;

function mcfg_add_section(sector: PMcfgSector; name: PChar): TMcfgErr; cdecl; external;

function mcfg_add_dynfield(_file: PMcfgFile;
						   _type: TMcfgFieldType;
						   name: PChar;
						   data: Pointer;
						   size: SizeUInt): TMcfgErr; cdecl; external;

function mcfg_add_field(section: PMcfgSection;
						  _type: TMcfgFieldType;
						  name: PChar;
						  data: Pointer;
						  size: SizeUInt): TMcfgErr; cdecl; external;

function mcfg_add_list_field(list: PMcfgList; size: SizeUInt; data: Pointer): TMcfgErr; cdecl; external;

function mcfg_get_sector(_file: PMcfgFile; name: PChar): PMcfgSector; cdecl; external;

function mcfg_get_section(sector: PMcfgSector; name: PChar): PMcfgSection; cdecl; external;

function mcfg_get_dynfield(_file: PMcfgFile; name: PChar): PMcfgField; cdecl; external;

function mcfg_get_field(section: PMcfgSection; name: PChar): PMcfgField; cdecl; external;

procedure mcfg_free_list(list: TMcfgList); cdecl; external;

procedure mcfg_free_field(field: TMcfgField); cdecl; external;

procedure mcfg_free_section(section: TMcfgSection); cdecl; external;

procedure mcfg_free_sector(sector: TMcfgSector); cdecl; external;

procedure mcfg_free_file(_file: TMcfgFile); cdecl; external;

{ parser api }

function mcfg_parse(input: PChar): TMcfgParseResult; cdecl; external;

function mcfg_parse_from_file(path: PChar): TMcfgParseResult; cdecl; external;

{ serializer api }

function mcfg_serialize(_file: TMcfgFile; options: TMcfgSerializeOptions): TMcfgSerializeResult; cdecl; external;

const
	MCFG_2_VERSION = '0.5.0 (develop)';
	MCFG_DEFAULT_SERIALIZE_OPTIONS: TMcfgSerializeOptions = (tab_indentation: true; space_count: 0);

implementation

end.