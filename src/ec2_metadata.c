/*
 * ec2_metadata.c
 *
 *  Created on: Oct 5, 2016
 *      Author: carpenlc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <curl/curl.h>
#include "jsmn.h"
#include "ec2_metadata.h"

/*
 * Construct the EC2 instance metadata URL for retrieving the AWS credentials
 * associated with the IAM role.
 */
char *get_ec2_metadata_url (char *iam_role)
{
	// Determine the size of the required buffer
	size_t url_length = strlen(EC2_INSTANCE_META_DATA_URL)
				+ strlen(EC2_IAM_ROLE_OFFSET)
				+ strlen(iam_role)
				+ 1;

	char *url =  (char *)malloc(url_length);
	strcpy(url, EC2_INSTANCE_META_DATA_URL);
	strcat(url, EC2_IAM_ROLE_OFFSET);
	strcat(url, iam_role);
	return url;
}

/*
 * Convert a time string retrieved from AWS into a time_t type.
 * This method was taken from the s3fs project.
 */
time_t convert_cred_expiration_string(const char *aws_time)
{
    	struct tm tm;
	if (!aws_time)
	{
		return 0L;
	}
	memset(&tm, 0, sizeof(struct tm));
	strptime(aws_time, "%Y-%m-%dT%H:%M:%S", &tm);
	return timegm(&ym);
}

/*
 * Callback function used to copy the contents of the curl GET request
 * (i.e. JSON data) into allocated memory for further processing.
 */
static size_t curl_response_callback(
		void *contents,
		size_t size,
		size_t nmemb,
		void *response)
{
	size_t actual_size = size * nmemb;
	memory_structure *mem = (memory_structure *)response;
	mem->memory = realloc(mem->memory, mem->size + actual_size + 1);

	memcpy(&(mem->memory[mem->size]), contents, actual_size);
	if (mem->memory == NULL) {
		// TODO: Add logging reference here.
		fprintf(stderr, "Out of memory.  malloc(): errno=%d\n", errno);
		return 0;
	}
	mem->size += actual_size;
	mem->memory[mem->size] = 0;
	return actual_size;
}

/*
 * Function used to determine if the input JSON token matches the field name
 * that we're looking for.
 *
 * returns 0 if a match is found, -1 if no match.
 */
static int json_equals(const char *json, jsmntok_t *token, const char *field)
{
	if ((token->type == JSMN_STRING) &&
			((int)strlen(field) == token->end - token->start) &&
			(strncmp(json + token->start, field, token->end - token->start) == 0))
	{
		return 0;
	}
	return -1;
}

/*
 * This function takes the allocated memory containing the JSON response from
 * the cURL GET method and extracts the AWS credentials.
 *
 * This method assumes the availability of the jsmn (i.e. "jasmine") JSON
 * parser library which is embedded with the project.
 */
int parse_json_response(aws_credentials *creds, char *response)
{

	int i; // counter
	int r;
	jsmn_parser parser;
	jsmntok_t tokens[JSON_AWS_TOKEN_COUNT];

	// Initialize the jasmine JSON parser
	jsmn_init(&parser);

	// Parse the input JSON data.
	r = jsmn_parse(
			&parser,
			response,
			strlen(response),
			tokens,
			JSON_AWS_TOKEN_COUNT);

	if (r < 0)
	{
		printf("Failed to parse the JSON response [ %i ].", r);
		return -1;
	}

	// Loop through the tokens and extract the data of interest.
	for (i=1; i<r; i++)
	{
		if (json_equals(response, &tokens[i], JSON_AWS_CREDENTIAL_CREATED) == 0)
		{
			creds->last_updated = malloc(
					tokens[i+1].end - tokens[i+1].start + 1);
			strncpy(creds->last_updated,
					&response[tokens[i+1].start],
					tokens[i+1].end - tokens[i+1].start);
			i++;
		}
		else if (json_equals(response, &tokens[i], JSON_AWS_ACCESS_KEY) == 0)
		{
			creds->aws_access_key = malloc(
					sizeof(char*)*(tokens[i+1].end - tokens[i+1].start + 1));
			strncpy(creds->aws_access_key,
					&response[tokens[i+1].start],
					tokens[i+1].end - tokens[i+1].start);
			i++;
		}
		else if (json_equals(response, &tokens[i], JSON_AWS_SECRET_ACCESS_KEY) == 0)
		{
			creds->aws_secret_access_key = malloc(
					sizeof(char*)*(tokens[i+1].end - tokens[i+1].start + 1));
			strncpy(creds->aws_secret_access_key,
					&response[tokens[i+1].start],
					tokens[i+1].end - tokens[i+1].start);
			i++;
		}
		else if (json_equals(response, &tokens[i], JSON_AWS_TOKEN) == 0)
		{
			creds->aws_session_token = malloc(
					sizeof(char*)*(tokens[i+1].end - tokens[i+1].start + 1));
			strncpy(creds->aws_session_token,
					&response[tokens[i+1].start],
					tokens[i+1].end - tokens[i+1].start);
			i++;
		}
		else if (json_equals(response, &tokens[i], JSON_AWS_CREDENTIAL_EXPIRATION) == 0)
		{
			creds->expiration = malloc(
					sizeof(char*)*(tokens[i+1].end - tokens[i+1].start + 1));
			strncpy(creds->expiration,
					&response[tokens[i+1].start],
					tokens[i+1].end - tokens[i+1].start);
			i++;
		}
	}

	return 0;
}

/*
 * Pre-process the JSON data for input into the jsmn JSON parser.  The
 * jsmn JSON parser does not handle white space well.
 */
void remove_whitespace(char *json)
{
	int i = 0;
	int count = 0;
	for (i = 0; json[i]; i++)
	{
		if ((json[i] != ' ')
				&& (json[i] != '\n'))
		{
			json[count++] = json[i];
		}
	}
	json[count] = '\0';
}

void get_aws_credentials(aws_credentials *creds, char *iam_role) {

	char *url;
	memory_structure response;
	CURL *curl_handle;
	CURLcode result;

	// Initialize the memory that will be used to store the JSON data
	// returned in the cURL response.
	response.memory = malloc(1);
	response.size = 0;

	// Get the target URL
	url = get_ec2_metadata_url(iam_role);

	// Initialize the cURL library
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_response_callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	// Execute the cURL command
	result = curl_easy_perform(curl_handle);
	if (result != CURLE_OK)
	{
        	printf("Error performing cURL operation.  Response [ %i ].", result);
	}
	else
	{
		printf("cURL operation successful [ %s ]", response.memory);
		remove_whitespace(response.memory);
		parse_json_response(creds, response.memory);
	}

	// Cleanup and release memory.
	curl_easy_cleanup(curl_handle);
	free(response.memory);
	curl_global_cleanup();
}

/*
 * Test method used to output the credentials extracted from AWS.
 */
void print_aws_credentials(aws_credentials *creds)
{
	printf("\nLast Updated [ %s ]\n", creds->last_updated);
	printf("Access Key [ %s ]\n", creds->aws_access_key);
	printf("Secret Access Key [ %s ]\n", creds->aws_secret_access_key);
	printf("Token [ %s ]\n", creds->aws_session_token);
	printf("Expiration [ %s ]\n", creds->expiration);
}

void update_aws_credentials(Application *app)
{
	
}

int main(void)
{

	aws_credentials *creds;
	if (creds == NULL) {
		creds = malloc(sizeof(*creds));
	}
	get_aws_credentials(creds, "S3FileServer");
	print_aws_credentials(creds);

}


