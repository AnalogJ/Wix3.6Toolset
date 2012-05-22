/***************************************************************************

Copyright (c) Microsoft Corporation. All rights reserved.
This code is licensed under the Visual Studio SDK license terms.
THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.

***************************************************************************/

using Microsoft.VisualStudio.Shell.Interop;
using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using System.Diagnostics;
using System.Globalization;
using System.Collections;
using System.IO;
using MSBuild = Microsoft.Build.BuildEngine;
using System.Collections.Generic;
using EnvDTE;
using System.Diagnostics.CodeAnalysis;


/* This file provides a basefunctionallity for IVsCfgProvider2.
   Instead of using the IVsProjectCfgEventsHelper object we have our own little sink and call our own helper methods
   similiar to the interface. But there is no real benefit in inheriting from the interface in the first place. 
   Using the helper object seems to be:  
    a) undocumented
    b) not really wise in the managed world
*/
namespace Microsoft.VisualStudio.Package
{

    [CLSCompliant(false)]
    [ComVisible(true)]
    public class ConfigProvider : IVsCfgProvider2, IVsProjectCfgProvider, IVsExtensibleObject
    {
        #region fields
        internal const string configString = " '$(Configuration)' == '{0}' ";
        internal const string AnyCPUPlatform = "Any CPU";

        private ProjectNode project;
        private EventSinkCollection cfgEventSinks = new EventSinkCollection();
        private List<KeyValuePair<KeyValuePair<string, string>, string>> newCfgProps = new List<KeyValuePair<KeyValuePair<string, string>, string>>();
        protected Dictionary<string, ProjectConfig> configurationsList = new Dictionary<string, ProjectConfig>();
        #endregion

        #region Properties
        /// <summary>
        /// The associated project.
        /// </summary>
        protected ProjectNode ProjectMgr
        {
            get
            {
                return this.project;
            }
        }
        /// <summary>
        /// If the project system wants to add custom properties to the property group then 
        /// they provide us with this data.
        /// Returns/sets the [(<propName, propCondition>) <propValue>] collection
        /// </summary>
        public virtual List<KeyValuePair<KeyValuePair<string, string>, string>> NewConfigProperties
        {
            get
            {
                return newCfgProps;
            }
            set
            {
                newCfgProps = value;
            }
        }

        #endregion

        #region ctors
        public ConfigProvider(ProjectNode manager)
        {
            this.project = manager;
        }
        #endregion

        #region methods
        /// <summary>
        /// Invalidates any cache of properties after some property value has changed.
        /// </summary>
        public virtual void InvalidatePropertyCache()
        {
            foreach (ProjectConfig projectConfig in this.configurationsList.Values)
            {
                projectConfig.InvalidatePropertyCache();
            }
        }

        /// <summary>
        /// Creates new Project Configuartion objects based on the configuration name.
        /// </summary>
        /// <param name="configName">The name of the configuration</param>
        /// <returns>An instance of a ProjectConfig object.</returns>
        protected ProjectConfig GetProjectConfiguration(string configName)
        {
            // if we already created it, return the cached one
            if (configurationsList.ContainsKey(configName))
            {
                return configurationsList[configName];
            }

            ProjectConfig requestedConfiguration = CreateProjectConfiguration(configName);
            configurationsList.Add(configName, requestedConfiguration);

            return requestedConfiguration;
        }

        protected virtual ProjectConfig CreateProjectConfiguration(string configName)
        {
            return new DebuggableProjectConfig(this.project, configName);
        }

        #endregion

        #region IVsProjectCfgProvider methods
        /// <summary>
        /// Provides access to the IVsProjectCfg interface implemented on a project's configuration object. 
        /// </summary>
        /// <param name="projectCfgCanonicalName">The canonical name of the configuration to access.</param>
        /// <param name="projectCfg">The IVsProjectCfg interface of the configuration identified by szProjectCfgCanonicalName.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code. </returns>
        public virtual int OpenProjectCfg(string projectCfgCanonicalName, out IVsProjectCfg projectCfg)
        {
            if (projectCfgCanonicalName == null)
            {
                throw new ArgumentNullException("projectCfgCanonicalName");
            }

            projectCfg = null;

            // Be robust in release
            if (projectCfgCanonicalName == null)
            {
                return VSConstants.E_INVALIDARG;
            }


            Debug.Assert(this.project != null && this.project.BuildProject != null);

            string[] configs = this.project.BuildProject.GetConditionedPropertyValues(ProjectFileConstants.Configuration);


            foreach (string config in configs)
            {
                if (String.Compare(config, projectCfgCanonicalName, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    projectCfg = this.GetProjectConfiguration(config);
                    if (projectCfg != null)
                    {
                        return VSConstants.S_OK;
                    }
                    else
                    {
                        return VSConstants.E_FAIL;
                    }
                }
            }

            return VSConstants.E_INVALIDARG;
        }

        /// <summary>
        /// Checks whether or not this configuration provider uses independent configurations. 
        /// </summary>
        /// <param name="usesIndependentConfigurations">true if independent configurations are used, false if they are not used. By default returns true.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int get_UsesIndependentConfigurations(out int usesIndependentConfigurations)
        {
            usesIndependentConfigurations = 1;
            return VSConstants.S_OK;
        }
        #endregion

        #region IVsCfgProvider2 methods
        /// <summary>
        /// Copies an existing configuration name or creates a new one. 
        /// </summary>
        /// <param name="name">The name of the new configuration.</param>
        /// <param name="cloneName">the name of the configuration to copy, or a null reference, indicating that AddCfgsOfCfgName should create a new configuration.</param>
        /// <param name="fPrivate">Flag indicating whether or not the new configuration is private. If fPrivate is set to true, the configuration is private. If set to false, the configuration is public. This flag can be ignored.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code. </returns>
        public virtual int AddCfgsOfCfgName(string name, string cloneName, int fPrivate)
        {
            // We need to QE/QS the project file
            if (!this.ProjectMgr.QueryEditProjectFile(false))
            {
                throw Marshal.GetExceptionForHR(VSConstants.OLE_E_PROMPTSAVECANCELLED);
            }

            // First create the condition that represent the configuration we want to clone
            string condition = (cloneName == null ? String.Empty : String.Format(CultureInfo.InvariantCulture, configString, cloneName).Trim());

            // Get all configs
            MSBuild.BuildPropertyGroupCollection configGroup = this.project.BuildProject.PropertyGroups;
            MSBuild.BuildPropertyGroup configToClone = null;

            if (cloneName != null)
            {
                // Find the configuration to clone
                foreach (MSBuild.BuildPropertyGroup currentConfig in configGroup)
                {
                    // Only care about conditional property groups
                    if (currentConfig.Condition == null || currentConfig.Condition.Length == 0)
                        continue;

                    // Skip if it isn't the group we want
                    if (String.Compare(currentConfig.Condition.Trim(), condition, StringComparison.OrdinalIgnoreCase) != 0)
                        continue;

                    configToClone = currentConfig;
                }
            }

            MSBuild.BuildPropertyGroup newConfig = null;
            if (configToClone != null)
            {
                // Clone the configuration settings
                newConfig = this.project.ClonePropertyGroup(configToClone);
                //Will be added later with the new values to the path
                newConfig.RemoveProperty("OutputPath");
            }
            else
            {
                // no source to clone from, lets just create a new empty config
                newConfig = this.project.BuildProject.AddNewPropertyGroup(false);
                // Get the list of property name, condition value from the config provider
                IList<KeyValuePair<KeyValuePair<string, string>, string>> propVals = this.NewConfigProperties;
                foreach (KeyValuePair<KeyValuePair<string, string>, string> data in propVals)
                {
                    KeyValuePair<string, string> propData = data.Key;
                    string value = data.Value;
                    MSBuild.BuildProperty newProperty = newConfig.AddNewProperty(propData.Key, value);
                    if (!String.IsNullOrEmpty(propData.Value))
                        newProperty.Condition = propData.Value;
                }
            }


            //add the output path
            string outputBasePath = this.ProjectMgr.OutputBaseRelativePath;
            if (outputBasePath.EndsWith(Path.DirectorySeparatorChar.ToString(), StringComparison.Ordinal))
                outputBasePath = Path.GetDirectoryName(outputBasePath);
            newConfig.AddNewProperty("OutputPath", Path.Combine(outputBasePath, name) + Path.DirectorySeparatorChar.ToString());

            // Set the condition that will define the new configuration
            string newCondition = String.Format(CultureInfo.InvariantCulture, configString, name);
            newConfig.Condition = newCondition;

            NotifyOnCfgNameAdded(name);
            return VSConstants.S_OK;
        }

        /// <summary>
        /// Copies an existing platform name or creates a new one. 
        /// </summary>
        /// <param name="platformName">The name of the new platform.</param>
        /// <param name="clonePlatformName">The name of the platform to copy, or a null reference, indicating that AddCfgsOfPlatformName should create a new platform.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int AddCfgsOfPlatformName(string platformName, string clonePlatformName)
        {
            return VSConstants.E_NOTIMPL;
        }

        /// <summary>
        /// Deletes a specified configuration name. 
        /// </summary>
        /// <param name="name">The name of the configuration to be deleted.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code. </returns>
        public virtual int DeleteCfgsOfCfgName(string name)
        {
            // We need to QE/QS the project file
            if (!this.ProjectMgr.QueryEditProjectFile(false))
            {
                throw Marshal.GetExceptionForHR(VSConstants.OLE_E_PROMPTSAVECANCELLED);
            }

            if (name == null)
            {
                Debug.Fail(String.Format(CultureInfo.CurrentCulture, "Name of the configuration should not be null if you want to delete it from project: {0}", this.project.BuildProject.FullFileName));
                // The configuration " '$(Configuration)' ==  " does not exist, so technically the goal
                // is achieved so return S_OK
                return VSConstants.S_OK;
            }
            // Verify that this config exist
            string[] configs = this.project.BuildProject.GetConditionedPropertyValues(ProjectFileConstants.Configuration);
            foreach (string config in configs)
            {
                if (String.Compare(config, name, StringComparison.OrdinalIgnoreCase) == 0)
                {
                    // Create condition of config to remove
                    string condition = String.Format(CultureInfo.InvariantCulture, configString, config);
                    this.project.BuildProject.RemovePropertyGroupsWithMatchingCondition(condition);

                    NotifyOnCfgNameDeleted(name);
                }
            }

            return VSConstants.S_OK;
        }

        /// <summary>
        /// Deletes a specified platform name. 
        /// </summary>
        /// <param name="platName">The platform name to delet.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int DeleteCfgsOfPlatformName(string platName)
        {
            return VSConstants.E_NOTIMPL;
        }

        /// <summary>
        /// Returns the existing configurations stored in the project file.
        /// </summary>
        /// <param name="celt">Specifies the requested number of property names. If this number is unknown, celt can be zero.</param>
        /// <param name="names">On input, an allocated array to hold the number of configuration property names specified by celt. This parameter can also be a null reference if the celt parameter is zero. 
        /// On output, names contains configuration property names.</param>
        /// <param name="actual">The actual number of property names returned.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int GetCfgNames(uint celt, string[] names, uint[] actual)
        {
            // get's called twice, once for allocation, then for retrieval            
            int i = 0;

            string[] configList = this.project.BuildProject.GetConditionedPropertyValues(ProjectFileConstants.Configuration);

            if (names != null)
            {
                foreach (string config in configList)
                {
                    names[i++] = config;
                    if (i == celt)
                        break;
                }
            }
            else
                i = configList.Length;

            if (actual != null)
            {
                actual[0] = (uint)i;
            }

            return VSConstants.S_OK;
        }

        /// <summary>
        /// Returns the configuration associated with a specified configuration or platform name. 
        /// </summary>
        /// <param name="name">The name of the configuration to be returned.</param>
        /// <param name="platName">The name of the platform for the configuration to be returned.</param>
        /// <param name="cfg">The implementation of the IVsCfg interface.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int GetCfgOfName(string name, string platName, out IVsCfg cfg)
        {
            cfg = null;
            cfg = this.GetProjectConfiguration(name);

            return VSConstants.S_OK;
        }

        /// <summary>
        /// Returns a specified configuration property. 
        /// </summary>
        /// <param name="propid">Specifies the property identifier for the property to return. For valid propid values, see __VSCFGPROPID.</param>
        /// <param name="var">The value of the property.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int GetCfgProviderProperty(int propid, out object var)
        {
            var = false;
            switch ((__VSCFGPROPID)propid)
            {
                case __VSCFGPROPID.VSCFGPROPID_SupportsCfgAdd:
                    var = true;
                    break;

                case __VSCFGPROPID.VSCFGPROPID_SupportsCfgDelete:
                    var = true;
                    break;

                case __VSCFGPROPID.VSCFGPROPID_SupportsCfgRename:
                    var = true;
                    break;

                case __VSCFGPROPID.VSCFGPROPID_SupportsPlatformAdd:
                    var = false;
                    break;

                case __VSCFGPROPID.VSCFGPROPID_SupportsPlatformDelete:
                    var = false;
                    break;
            }
            return VSConstants.S_OK;
        }

        /// <summary>
        /// Returns the per-configuration objects for this object. 
        /// </summary>
        /// <param name="celt">Number of configuration objects to be returned or zero, indicating a request for an unknown number of objects.</param>
        /// <param name="a">On input, pointer to an interface array or a null reference. On output, this parameter points to an array of IVsCfg interfaces belonging to the requested configuration objects.</param>
        /// <param name="actual">The number of configuration objects actually returned or a null reference, if this information is not necessary.</param>
        /// <param name="flags">Flags that specify settings for project configurations, or a null reference (Nothing in Visual Basic) if no additional flag settings are required. For valid prgrFlags values, see __VSCFGFLAGS.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int GetCfgs(uint celt, IVsCfg[] a, uint[] actual, uint[] flags)
        {
            if (flags != null)
                flags[0] = 0;

            int i = 0;
            string[] configList = this.project.BuildProject.GetConditionedPropertyValues(ProjectFileConstants.Configuration);

            if (a != null)
            {
                foreach (string configName in configList)
                {
                    a[i] = this.GetProjectConfiguration(configName);

                    i++;
                    if (i == celt)
                        break;
                }
            }
            else
                i = configList.Length;

            if (actual != null)
                actual[0] = (uint)i;

            return VSConstants.S_OK;
        }

        /// <summary>
        /// Returns one or more platform names. 
        /// </summary>
        /// <param name="celt">Specifies the requested number of platform names. If this number is unknown, celt can be zero.</param>
        /// <param name="names">On input, an allocated array to hold the number of platform names specified by celt. This parameter can also be a null reference if the celt parameter is zero. On output, names contains platform names.</param>
        /// <param name="actual">The actual number of platform names returned.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int GetPlatformNames(uint celt, string[] names, uint[] actual)
        {
            string[] platforms = this.GetPlatformsFromProject();
            return GetPlatforms(celt, names, actual, platforms);
        }

        /// <summary>
        /// Returns the set of platforms that are installed on the user's machine. 
        /// </summary>
        /// <param name="celt">Specifies the requested number of supported platform names. If this number is unknown, celt can be zero.</param>
        /// <param name="names">On input, an allocated array to hold the number of names specified by celt. This parameter can also be a null reference (Nothing in Visual Basic)if the celt parameter is zero. On output, names contains the names of supported platforms</param>
        /// <param name="actual">The actual number of platform names returned.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int GetSupportedPlatformNames(uint celt, string[] names, uint[] actual)
        {
            string[] platforms = this.GetSupportedPlatformsFromProject();
            return GetPlatforms(celt, names, actual, platforms);
        }

        /// <summary>
        /// Assigns a new name to a configuration. 
        /// </summary>
        /// <param name="old">The old name of the target configuration.</param>
        /// <param name="newname">The new name of the target configuration.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int RenameCfgsOfCfgName(string old, string newname)
        {
            // First create the condition that represent the configuration we want to rename
            string condition = String.Format(CultureInfo.InvariantCulture, configString, old).Trim();

            foreach (MSBuild.BuildPropertyGroup config in this.project.BuildProject.PropertyGroups)
            {
                // Only care about conditional property groups
                if (config.Condition == null || config.Condition.Length == 0)
                    continue;

                // Skip if it isn't the group we want
                if (String.Compare(config.Condition.Trim(), condition, StringComparison.OrdinalIgnoreCase) != 0)
                    continue;

                // Change the name 
                config.Condition = String.Format(CultureInfo.InvariantCulture, configString, newname);
                // Update the name in our config list
                if (configurationsList.ContainsKey(old))
                {
                    ProjectConfig configuration = configurationsList[old];
                    configurationsList.Remove(old);
                    configurationsList.Add(newname, configuration);
                    // notify the configuration of its new name
                    configuration.ConfigName = newname;
                }

                NotifyOnCfgNameRenamed(old, newname);
            }

            return VSConstants.S_OK;
        }

        /// <summary>
        /// Cancels a registration for configuration event notification. 
        /// </summary>
        /// <param name="cookie">The cookie used for registration.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int UnadviseCfgProviderEvents(uint cookie)
        {
            this.cfgEventSinks.RemoveAt(cookie);
            return VSConstants.S_OK;
        }

        /// <summary>
        /// Registers the caller for configuration event notification. 
        /// </summary>
        /// <param name="sink">Reference to the IVsCfgProviderEvents interface to be called to provide notification of configuration events.</param>
        /// <param name="cookie">Reference to a token representing the completed registration</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        public virtual int AdviseCfgProviderEvents(IVsCfgProviderEvents sink, out uint cookie)
        {
            cookie = this.cfgEventSinks.Add(sink);
            return VSConstants.S_OK;
        }
        #endregion

        #region IVsExtensibleObject Members

        /// <summary>
        /// Proved access to an IDispatchable object being a list of configuration properties
        /// </summary>
        /// <param name="configurationName">Combined Name and Platform for the configuration requested</param>
        /// <param name="configurationProperties">The IDispatchcable object</param>
        /// <returns>S_OK if successful</returns>
        public virtual int GetAutomationObject(string configurationName, out object configurationProperties)
        {
            //Init out param
            configurationProperties = null;

            string name, platform;
            if (!ProjectConfig.TrySplitConfigurationCanonicalName(configurationName, out name, out platform))
            {
                return VSConstants.E_INVALIDARG;
            }

            // Get the configuration
            IVsCfg cfg;
            ErrorHandler.ThrowOnFailure(this.GetCfgOfName(name, platform, out cfg));

            // Get the properties of the configuration
            configurationProperties = ((ProjectConfig)cfg).ConfigurationProperties;

            return VSConstants.S_OK;

        }
        #endregion

        #region helper methods
        /// <summary>
        /// Called when a new configuration name was added.
        /// </summary>
        /// <param name="name">The name of configuration just added.</param>
        protected void NotifyOnCfgNameAdded(string name)
        {
            foreach (IVsCfgProviderEvents sink in this.cfgEventSinks)
            {
                ErrorHandler.ThrowOnFailure(sink.OnCfgNameAdded(name));
            }
        }

        /// <summary>
        /// Called when a config name was deleted.
        /// </summary>
        /// <param name="name">The name of the configuration.</param>
        protected void NotifyOnCfgNameDeleted(string name)
        {
            foreach (IVsCfgProviderEvents sink in this.cfgEventSinks)
            {
                ErrorHandler.ThrowOnFailure(sink.OnCfgNameDeleted(name));
            }
        }

        /// <summary>
        /// Called when a config name was renamed
        /// </summary>
        /// <param name="oldName">Old configuration name</param>
        /// <param name="newName">New configuration name</param>
        protected void NotifyOnCfgNameRenamed(string oldName, string newName)
        {
            foreach (IVsCfgProviderEvents sink in this.cfgEventSinks)
            {
                ErrorHandler.ThrowOnFailure(sink.OnCfgNameRenamed(oldName, newName));
            }
        }

        /// <summary>
        /// Called when a platform name was added
        /// </summary>
        /// <param name="platformName">The name of the platform.</param>
        [SuppressMessage("Microsoft.Performance","CA1811:AvoidUncalledPrivateCode")]
        protected void NotifyOnPlatformNameAdded(string platformName)
        {
            foreach (IVsCfgProviderEvents sink in this.cfgEventSinks)
            {
                ErrorHandler.ThrowOnFailure(sink.OnPlatformNameAdded(platformName));
            }
        }

        /// <summary>
        /// Called when a platform name was deleted
        /// </summary>
        /// <param name="platformName">The name of the platform.</param>
        [SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
        protected void NotifyOnPlatformNameDeleted(string platformName)
        {
            foreach (IVsCfgProviderEvents sink in this.cfgEventSinks)
            {
                ErrorHandler.ThrowOnFailure(sink.OnPlatformNameDeleted(platformName));
            }
        }

        /// <summary>
        /// Gets all the platforms defined in the project
        /// </summary>
        /// <returns>An array of platform names.</returns>
        private string[] GetPlatformsFromProject()
        {
            string[] platforms = this.ProjectMgr.BuildProject.GetConditionedPropertyValues(ProjectFileConstants.Platform);

            if (platforms == null || platforms.Length == 0)
            {
                return new string[] { AnyCPUPlatform };
            }

            for (int i = 0; i < platforms.Length; i++)
            {
                platforms[i] = ConvertPlatformToVsProject(platforms[i]);
            }

            return platforms;
        }

        /// <summary>
        /// Return the supported platform names.
        /// </summary>
        /// <returns>An array of supported platform names.</returns>
        private string[] GetSupportedPlatformsFromProject()
        {
            string platforms = this.ProjectMgr.BuildProject.GetEvaluatedProperty(ProjectFileConstants.AvailablePlatforms);

            if (platforms == null)
            {
                return new string[] { };
            }

            if (platforms.Contains(","))
            {
                return platforms.Split(',');
            }

            return new string[] { platforms };
        }

        /// <summary>
        /// Helper function to convert AnyCPU to Any CPU.
        /// </summary>
        /// <param name="oldName">The oldname.</param>
        /// <returns>The new name.</returns>
        private static string ConvertPlatformToVsProject(string oldPlatformName)
        {
            if (String.Compare(oldPlatformName, ProjectFileValues.AnyCPU, StringComparison.OrdinalIgnoreCase) == 0)
            {
                return AnyCPUPlatform;
            }

            return oldPlatformName;
        }

        /// <summary>
        /// Common method for handling platform names.
        /// </summary>
        /// <param name="celt">Specifies the requested number of platform names. If this number is unknown, celt can be zero.</param>
        /// <param name="names">On input, an allocated array to hold the number of platform names specified by celt. This parameter can also be null if the celt parameter is zero. On output, names contains platform names</param>
        /// <param name="actual">A count of the actual number of platform names returned.</param>
        /// <param name="platforms">An array of available platform names</param>
        /// <returns>A count of the actual number of platform names returned.</returns>
        /// <devremark>The platforms array is never null. It is assured by the callers.</devremark>
        protected static int GetPlatforms(uint celt, string[] names, uint[] actual, string[] platforms)
        {
            Debug.Assert(platforms != null, "The plaforms array should never be null");
            if (names == null)
            {
                if (actual == null || actual.Length == 0)
                {
                    throw new ArgumentException(SR.GetString(SR.InvalidParameter, CultureInfo.CurrentUICulture), "actual");
                }

                actual[0] = (uint)platforms.Length;
                return VSConstants.S_OK;
            }

            //Degenarate case
            if (celt == 0)
            {
                if (actual != null && actual.Length != 0)
                {
                    actual[0] = (uint)platforms.Length;
                }

                return VSConstants.S_OK;
            }

            uint returned = 0;
            for (int i = 0; i < platforms.Length && names.Length > returned; i++)
            {
                names[returned] = platforms[i];
                returned++;
            }

            if (actual != null && actual.Length != 0)
            {
                actual[0] = returned;
            }

            if (celt > returned)
            {
                return VSConstants.S_FALSE;
            }

            return VSConstants.S_OK;
        }
        #endregion
    }
}
