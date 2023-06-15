// Copyright (c) 2021 Josef Prenner
// < support@ludicdrive.com >
// ludicdrive.com
// All Rights Reserved

using System.IO;
using UnrealBuildTool;
using System.Collections.Generic;

public class Landscaping : ModuleRules
{
	public Landscaping(ReadOnlyTargetRules Target) : base(Target)
	{
		bEnableUndefinedIdentifierWarnings = false;
		
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore", 
				"UnrealEd",
				"BSPUtils",
				"MeshDescription",
				"StaticMeshDescription",
				"WebBrowser",
				"GeoReferencing",
				"Json",
				"JsonUtilities"
			});

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"AssetRegistry",
				"AssetTools",
				"ContentBrowser",
				"Landscape"
			}
		);
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"Landscape",
				"LevelEditor",
				"PropertyEditor",
				"DesktopPlatform",
				"MainFrame",
				"SourceControl",
				"SourceControlWindows",
				"NewLevelDialog",
				"LandscapeEditor",
				"FoliageEdit",
				"Foliage",
				"MaterialUtilities",
				"EditorWidgets",
				"Blutility",
				"EditorSubsystem",
				"ProceduralMeshComponent"
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"AssetRegistry",
				"AssetTools",
				"SceneOutliner",
				"ContentBrowser"
			}
		);

		// header files (platform independent)
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "Include"));
		
		
		// libs
		List<string> PublicAdditionalLibsNames = new List<string>();
		PublicAdditionalLibsNames.Add("cairo.lib");
		PublicAdditionalLibsNames.Add("cfitsio.lib");
		PublicAdditionalLibsNames.Add("fontconfig.lib");
		PublicAdditionalLibsNames.Add("freetype.lib");
		PublicAdditionalLibsNames.Add("freexl.lib");
		PublicAdditionalLibsNames.Add("freexl_i.lib");
		PublicAdditionalLibsNames.Add("fribidi.lib");
		PublicAdditionalLibsNames.Add("fts5.lib");
		PublicAdditionalLibsNames.Add("gdal_i.lib");
		PublicAdditionalLibsNames.Add("geos.lib");
		PublicAdditionalLibsNames.Add("geos_c.lib");
		PublicAdditionalLibsNames.Add("giflib.lib");
		PublicAdditionalLibsNames.Add("harfbuzz.lib");
		PublicAdditionalLibsNames.Add("hdf.lib");
		PublicAdditionalLibsNames.Add("hdf5.lib");
		PublicAdditionalLibsNames.Add("hdf5_cpp.lib");
		PublicAdditionalLibsNames.Add("hdf5_hl.lib");
		PublicAdditionalLibsNames.Add("hdf5_hl_cpp.lib");
		PublicAdditionalLibsNames.Add("iconv.lib");
		PublicAdditionalLibsNames.Add("kmlbase.lib");
		PublicAdditionalLibsNames.Add("kmlconvenience.lib");
		PublicAdditionalLibsNames.Add("kmldom.lib");
		PublicAdditionalLibsNames.Add("kmlengine.lib");
		PublicAdditionalLibsNames.Add("kmlregionator.lib");
		PublicAdditionalLibsNames.Add("kmlxsd.lib");
		PublicAdditionalLibsNames.Add("libcrypto.lib");
		PublicAdditionalLibsNames.Add("libcrypto_static.lib");
		PublicAdditionalLibsNames.Add("libcurl_imp.lib");
		PublicAdditionalLibsNames.Add("libexpat.lib");
		PublicAdditionalLibsNames.Add("libfcgi.lib");
		PublicAdditionalLibsNames.Add("libhdf.lib");
		PublicAdditionalLibsNames.Add("libhdf5.lib");
		PublicAdditionalLibsNames.Add("libhdf5_cpp.lib");
		PublicAdditionalLibsNames.Add("libhdf5_hl.lib");
		PublicAdditionalLibsNames.Add("libhdf5_hl_cpp.lib");
		PublicAdditionalLibsNames.Add("libhttpd.lib");
		PublicAdditionalLibsNames.Add("libjpeg.lib");
		PublicAdditionalLibsNames.Add("libkea.lib");
		PublicAdditionalLibsNames.Add("libmfhdf.lib");
		PublicAdditionalLibsNames.Add("libmysql.lib");
		PublicAdditionalLibsNames.Add("libpng16.lib");
		PublicAdditionalLibsNames.Add("libpng16_static.lib");
		PublicAdditionalLibsNames.Add("libpq.lib");
		PublicAdditionalLibsNames.Add("libprotobuf-lite.lib");
		PublicAdditionalLibsNames.Add("libprotobuf.lib");
		PublicAdditionalLibsNames.Add("libprotoc.lib");
		PublicAdditionalLibsNames.Add("librttopo.lib");
		PublicAdditionalLibsNames.Add("libsqlite3.lib");
		PublicAdditionalLibsNames.Add("libssl.lib");
		PublicAdditionalLibsNames.Add("libssl_static.lib");
		PublicAdditionalLibsNames.Add("libsvg-cairo.lib");
		PublicAdditionalLibsNames.Add("libsvg.lib");
		PublicAdditionalLibsNames.Add("libszip.lib");
		PublicAdditionalLibsNames.Add("libxdr.lib");
		PublicAdditionalLibsNames.Add("libxml2.lib");
		PublicAdditionalLibsNames.Add("mfhdf.lib");
		PublicAdditionalLibsNames.Add("minizip.lib");
		PublicAdditionalLibsNames.Add("netcdf.lib");
		PublicAdditionalLibsNames.Add("ogdi.lib");
		PublicAdditionalLibsNames.Add("openjp2.lib");
		PublicAdditionalLibsNames.Add("pcre.lib");
		PublicAdditionalLibsNames.Add("pcrecpp.lib");
		PublicAdditionalLibsNames.Add("pcreposix.lib");
		PublicAdditionalLibsNames.Add("pixman-1.lib");
		PublicAdditionalLibsNames.Add("poppler.lib");
		PublicAdditionalLibsNames.Add("proj.lib");
		PublicAdditionalLibsNames.Add("proj_4_9.lib");
		PublicAdditionalLibsNames.Add("proj_i.lib");
		PublicAdditionalLibsNames.Add("protobuf-c.lib");
		PublicAdditionalLibsNames.Add("spatialite.lib");
		PublicAdditionalLibsNames.Add("spatialite_i.lib");
		PublicAdditionalLibsNames.Add("sqlite3.lib");
		PublicAdditionalLibsNames.Add("sqlite3_i.lib");
		PublicAdditionalLibsNames.Add("szip.lib");
		PublicAdditionalLibsNames.Add("tiff.lib");
		PublicAdditionalLibsNames.Add("uriparser.lib");
		PublicAdditionalLibsNames.Add("webp.lib");
		PublicAdditionalLibsNames.Add("webpdecoder.lib");
		PublicAdditionalLibsNames.Add("webpdemux.lib");
		PublicAdditionalLibsNames.Add("webpmux.lib");
		PublicAdditionalLibsNames.Add("xdr.lib");
		PublicAdditionalLibsNames.Add("xerces-c_3.lib");
		PublicAdditionalLibsNames.Add("zlib.lib");
		PublicAdditionalLibsNames.Add("zlibstatic.lib");
		PublicAdditionalLibsNames.Add("zstd.lib");
		PublicAdditionalLibsNames.Add("zstd_static.lib");

		string LibFolder = Path.Combine(ModuleDirectory, "ThirdParty", "Lib");

		foreach (string PublicAdditionalLibsName in PublicAdditionalLibsNames)
		{
			string LibPath = Path.Combine(LibFolder, PublicAdditionalLibsName);
			if (!File.Exists(LibPath))
			{
				string Err = string.Format("Library '{0}' not found.", LibPath);
				System.Console.WriteLine(Err);
				throw new BuildException(Err);
			}
			PublicAdditionalLibraries.Add(LibPath);
		}
	
		// dlls
		List<string> RuntimeModuleNames = new List<string>();
		RuntimeModuleNames.Add("adrg.dll");
		RuntimeModuleNames.Add("cairo.dll");
		RuntimeModuleNames.Add("cfitsio.dll");
		RuntimeModuleNames.Add("concrt140.dll");
		RuntimeModuleNames.Add("FileGDBAPI.dll");
		RuntimeModuleNames.Add("freexl.dll");
		RuntimeModuleNames.Add("fribidi-0.dll");
		RuntimeModuleNames.Add("fts5.dll");
		RuntimeModuleNames.Add("gdal_ECW_JP2ECW.dll");
		RuntimeModuleNames.Add("gdal305.dll");
		RuntimeModuleNames.Add("geos.dll");
		RuntimeModuleNames.Add("geos_c.dll");
		RuntimeModuleNames.Add("hdf.dll");
		RuntimeModuleNames.Add("hdf5.dll");
		RuntimeModuleNames.Add("hdf5_cpp.dll");
		RuntimeModuleNames.Add("hdf5_hl.dll");
		RuntimeModuleNames.Add("hdf5_hl_cpp.dll");
		RuntimeModuleNames.Add("iconv-2.dll");
		RuntimeModuleNames.Add("libcrypto-1_1-x64.dll");
		RuntimeModuleNames.Add("libcurl.dll");
		RuntimeModuleNames.Add("libexpat.dll");
		RuntimeModuleNames.Add("libfcgi.dll");
		RuntimeModuleNames.Add("libhttpd.dll");
		RuntimeModuleNames.Add("libkea.dll");
		RuntimeModuleNames.Add("libmysql.dll");
		RuntimeModuleNames.Add("libpng16.dll");
		RuntimeModuleNames.Add("libpq.dll");
		RuntimeModuleNames.Add("librttopo.dll");
		RuntimeModuleNames.Add("libssl-1_1-x64.dll");
		RuntimeModuleNames.Add("libxml2.dll");
		RuntimeModuleNames.Add("lti_dsdk_9.5.dll");
		RuntimeModuleNames.Add("lti_lidar_dsdk_1.1.dll");
		RuntimeModuleNames.Add("mfhdf.dll");
		RuntimeModuleNames.Add("msvcp140.dll");
		RuntimeModuleNames.Add("msvcp140_1.dll");
		RuntimeModuleNames.Add("msvcp140_2.dll");
		RuntimeModuleNames.Add("msvcp140_atomic_wait.dll");
		RuntimeModuleNames.Add("msvcp140_codecvt_ids.dll");
		RuntimeModuleNames.Add("NCSEcw.dll");
		RuntimeModuleNames.Add("netcdf.dll");
		RuntimeModuleNames.Add("ogdi.dll");
		RuntimeModuleNames.Add("openjp2.dll");
		RuntimeModuleNames.Add("pcre.dll");
		RuntimeModuleNames.Add("pcrecpp.dll");
		RuntimeModuleNames.Add("pcreposix.dll");
		RuntimeModuleNames.Add("proj_4_9.dll");
		RuntimeModuleNames.Add("proj_7_2.dll");
		RuntimeModuleNames.Add("remote.dll");
		RuntimeModuleNames.Add("rpf.dll");
		RuntimeModuleNames.Add("skeleton.dll");
		RuntimeModuleNames.Add("spatialite.dll");
		RuntimeModuleNames.Add("sqlite3.dll");
		RuntimeModuleNames.Add("szip.dll");
		RuntimeModuleNames.Add("tbb.dll");
		RuntimeModuleNames.Add("tiff.dll");
		RuntimeModuleNames.Add("tiffxx.dll");
		RuntimeModuleNames.Add("vcruntime140.dll");
		RuntimeModuleNames.Add("vcruntime140_1.dll");
		RuntimeModuleNames.Add("vrf.dll");
		RuntimeModuleNames.Add("xdr.dll");
		RuntimeModuleNames.Add("xerces-c_3_2.dll");
		RuntimeModuleNames.Add("zlib.dll");
		RuntimeModuleNames.Add("zstd.dll");

		string ProjRedistFolder = Path.Combine(ModuleDirectory, "ThirdParty", "Redist", "Win64");

		foreach (string RuntimeModuleName in RuntimeModuleNames)
		{
			string ModulePath = Path.Combine(ProjRedistFolder, RuntimeModuleName);
			if (!File.Exists(ModulePath))
			{
				string Err = string.Format("Module '{0}' not found.", ModulePath);
				System.Console.WriteLine(Err);
				throw new BuildException(Err);
			}
			RuntimeDependencies.Add("$(BinaryOutputDir)/" + RuntimeModuleName, ModulePath);
		}

		// Stage GDAL data files
		RuntimeDependencies.Add("$(BinaryOutputDir)/gdal/*", Path.Combine(ProjRedistFolder, "gdal/*"), StagedFileType.SystemNonUFS);
		// Stage Proj data files
		RuntimeDependencies.Add("$(BinaryOutputDir)/proj/*", Path.Combine(ProjRedistFolder, "proj/*"), StagedFileType.SystemNonUFS);
		// Stage Proj7 data files
		RuntimeDependencies.Add("$(BinaryOutputDir)/proj7/*", Path.Combine(ProjRedistFolder, "proj7/*"), StagedFileType.SystemNonUFS);
	}
}
