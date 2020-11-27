using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Management;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace RCT
{
    public interface Comm
    {

        ManagementObject GetVirtualMachineSettings(ManagementObject virtualMachine);
        ManagementObject GetObject(string className, ManagementScope scope);
        ManagementObject GetFirstObjectFromCollection(ManagementObjectCollection collection);
        bool IsJobComplete(object jobStateObj);
        ManagementObject GetVmObject(string name, string className, ManagementScope scope);
        string GetReference(string vmName);
    };

    public class Class1 : Comm
    {
        enum JobState
        {
            New = 2,
            Starting = 3,
            Running = 4,
            Suspended = 5,
            ShuttingDown = 6,
            Completed = 7,
            Terminated = 8,
            Killed = 9,
            Exception = 10,
            CompletedWithWarnings = 32768
        }

        public bool IsJobComplete(object jobStateObj)
        {
            JobState jobState = (JobState)((ushort)jobStateObj);

            return (jobState == JobState.Completed) ||
                (jobState == JobState.CompletedWithWarnings) || (jobState == JobState.Terminated) ||
                (jobState == JobState.Exception) || (jobState == JobState.Killed);
        }
        private bool IsJobSuccessful(object jobStateObj)
        {
            JobState jobState = (JobState)((ushort)jobStateObj);

            return (jobState == JobState.Completed) || (jobState == JobState.CompletedWithWarnings);
        }

        public ManagementObject GetFirstObjectFromCollection(ManagementObjectCollection collection)
        {
            if (collection.Count == 0)
            {
                throw new ArgumentException("The collection contains no objects", "collection");
            }

            foreach (ManagementObject managementObject in collection)
            {
                return managementObject;
            }

            return null;
        }
        public ManagementObject GetObject(string className, ManagementScope scope)
        {
            using (ManagementClass snapshotServiceClass = new ManagementClass(className))
            {
                snapshotServiceClass.Scope = scope;

                ManagementObject snapshotService = GetFirstObjectFromCollection(snapshotServiceClass.GetInstances());

                return snapshotService;
            }
        }
        public ManagementObject
        GetVirtualMachineSettings(
            ManagementObject virtualMachine)
        {
            using (ManagementObjectCollection settingsCollection =
                    virtualMachine.GetRelated("Msvm_VirtualSystemSettingData", "Msvm_SettingsDefineState",
                    null, null, null, null, false, null))
            {
                ManagementObject virtualMachineSettings =
                    GetFirstObjectFromCollection(settingsCollection);

                return virtualMachineSettings;
            }
        }
        public ManagementObject
        GetVirtualMachineManagementService(
            ManagementScope scope)
        {
            using (ManagementClass managementServiceClass =
                new ManagementClass("Msvm_VirtualSystemManagementService"))
            {
                managementServiceClass.Scope = scope;

                ManagementObject managementService =
                    GetFirstObjectFromCollection(managementServiceClass.GetInstances());

                return managementService;
            }
        }
        public ManagementObject
        GetVmObject(
            string name,
            string className,
            ManagementScope scope)
        {
            string vmQueryWql = string.Format(CultureInfo.InvariantCulture,
                "SELECT * FROM {0} WHERE ElementName=\"{1}\"", className, name);

            SelectQuery vmQuery = new SelectQuery(vmQueryWql);

            using (ManagementObjectSearcher vmSearcher = new ManagementObjectSearcher(scope, vmQuery))
            using (ManagementObjectCollection vmCollection = vmSearcher.Get())
            {
                if (vmCollection.Count == 0)
                {
                    throw new ManagementException(string.Format(CultureInfo.CurrentCulture,
                        "No {0} could be found with name \"{1}\"",
                        className,
                        name));
                }

                //
                // If multiple virtual machines exist with the requested name, return the first 
                // one.
                //
                ManagementObject vm = GetFirstObjectFromCollection(vmCollection);

                return vm;
            }
        }
        public string GetReference(string vmName)
        {
            Class1 obj = new Class1();
            const UInt16 SnapshotTypeRecovery = 32768;
            Console.WriteLine("Type:{0}", SnapshotTypeRecovery);
            ManagementScope scope = new ManagementScope(@"\\.\root\virtualization\v2", null);
            ManagementObject virtualSystemService = obj.GetObject("Msvm_VirtualSystemSnapshotService", scope);
            /*ManagementObject virtualSystemSetting = GetObject("Msvm_VirtualSystemSettingData", scope);
            Console.WriteLine("SnapshotType:", virtualSystemSetting["VirtualSystemType"]);
            virtualSystemSetting["VirtualSystemType"] = "Microsoft:Hyper-V:Snapshot:Recovery";
            virtualSystemSetting.Put();*/

            ManagementBaseObject inParams = virtualSystemService.GetMethodParameters("CreateSnapshot");

            ManagementObject vm = obj.GetVmObject(vmName, "Msvm_ComputerSystem", scope);
            /*string query = String.Format("select * from Msvm_ComputerSystem where Caption=\"Virtual Machine\"");

            ManagementObjectSearcher searcher = new ManagementObjectSearcher(scope, new ObjectQuery(query));

            ManagementObjectCollection vms = searcher.Get();

            foreach (ManagementObject vm in vms)*/
            {
                ManagementObject vmSettings = obj.GetVirtualMachineSettings(vm);
                ManagementObject service = obj.GetVirtualMachineManagementService(scope);
                bool isEnabled = (bool)vmSettings["IncrementalBackupEnabled"];

                Console.WriteLine("Current Incremental bakcup status: {0}", isEnabled);

                if (isEnabled == false)
                {
                    vmSettings["IncrementalBackupEnabled"] = true;
                }
                ManagementBaseObject inParams2 = service.GetMethodParameters("ModifySystemSettings");
                inParams2["SystemSettings"] = vmSettings.GetText(TextFormat.CimDtd20);
                ManagementBaseObject outParams2 = service.InvokeMethod("ModifySystemSettings", inParams2, null);
                vmSettings = obj.GetVirtualMachineSettings(vm);
                isEnabled = (bool)vmSettings["IncrementalBackupEnabled"];

                Console.WriteLine("Current Incremental bakcup status: {0}", isEnabled);
                Console.WriteLine("ElementName:{0}", vm.Path.Path);
                inParams["AffectedSystem"] = vm.Path.Path;
                ManagementObject snapshotSettings = obj.GetObject("Msvm_VirtualSystemSnapshotSettingData", scope);
                snapshotSettings["ConsistencyLevel"] = 1;
                snapshotSettings["IgnoreNonSnapshottableDisks"] = true;
                snapshotSettings["GuestBackupType"] = 1;
                inParams["SnapshotSettings"] = snapshotSettings.GetText(TextFormat.WmiDtd20);

                // Set the SnapshotType property.
                inParams["SnapshotType"] = (UInt16)32768; // Recovery snapshot.
                ManagementBaseObject outParams = virtualSystemService.InvokeMethod("CreateSnapshot", inParams, null);
                ManagementObject job = new ManagementObject((string)outParams["Job"]);
                job.Scope = scope;
                if ((UInt32)outParams["ReturnValue"] == 4096)
                {
                    while (!obj.IsJobComplete(job["JobState"]))
                    {
                        Thread.Sleep(TimeSpan.FromSeconds(1));

                        // 
                        // ManagementObjects are offline objects. Call Get() on the object to have its
                        // current property state.
                        //
                        job.Get();
                    }
                }
                Console.WriteLine("Job:{0}", job);
                foreach (ManagementObject snap in job.GetRelated("Msvm_VirtualSystemSettingData"))
                {
                    Console.WriteLine("Resulting Snapshot:{0}", snap.Path.Path);
                    Console.WriteLine("Element:{0}", snap["ElementName"]);
                    Console.WriteLine("SnapshotType:{0}", snap["VirtualSystemType"]);
                    ManagementBaseObject inParams1 = virtualSystemService.GetMethodParameters("ConvertToReferencePoint");
                    inParams1["AffectedSnapshot"] = snap.Path.Path;
                    //inParams1["ReferencePointSettings"] = "";
                    ManagementBaseObject outParams1 = virtualSystemService.InvokeMethod("ConvertToReferencePoint", inParams1, null);
                    ManagementObject job1 = new ManagementObject((string)outParams1["Job"]);
                    job1.Scope = scope;
                    Console.WriteLine("ReturnValue:{0}", outParams1["ReturnValue"]);
                    if ((UInt32)outParams1["ReturnValue"] == 4096)
                    {
                        while (!IsJobComplete(job1["JobState"]))
                        {
                            Thread.Sleep(TimeSpan.FromSeconds(1));

                            // 
                            // ManagementObjects are offline objects. Call Get() on the object to have its
                            // current property state.
                            //
                            job1.Get();
                        }
                    }
                    foreach (ManagementObject reff in job1.GetRelated("Msvm_VirtualSystemReferencePoint"))
                    {
                        Console.WriteLine("Resulting Refference:{0}", reff.Path.Path);
                        string[] rct = (string[])reff["ResilientChangeTrackingIdentifiers"];
                        Console.WriteLine("RCT:{0}", rct[0]);
                        return rct[0];
                    }
                }


            }
            Console.ReadLine();
            return null;
        }
    }
}
