/*
 * ec2_metadata.h
 *
 *  Created on: Oct 5, 2016
 *      Author: carpenlc
 */

#ifndef INCLUDE_EC2_METADATA_H_
#define INCLUDE_EC2_METADATA_H_

/*
 * Memory structure used to hold data returned by the curl GET requests.
 */
typedef struct  {
    char *memory;
    size_t size;
} memory_structure;

/*
 * Memory structure used to hold the parsed AWS credential data.
 */
typedef struct {
    char *last_updated;
    char *aws_access_key;
    char *aws_secret_access_key;
    char *aws_session_token;
    char *expiration;
} aws_credentials;

/*
 * Base URL for the EC2 instance metadata.
 */
#define EC2_INSTANCE_META_DATA_URL "http://instance-data/latest/meta-data/"

/*
 * Offset URL context for retrieving the temporary IAM role credentials.
 */
#define EC2_IAM_ROLE_OFFSET "iam/security-credentials/"

/*
 * JSON element containing the time when the IAM role credentials were last
 * updated.
 */
#define JSON_AWS_CREDENTIAL_CREATED "LastUpdated"

/*
 * JSON element containing the time when the IAM role credentials are set to
 * expire.
 */
#define JSON_AWS_CREDENTIAL_EXPIRATION "Expiration"

/*
 * JSON element containing the AWS access key.
 */
#define JSON_AWS_ACCESS_KEY "AccessKeyId"

/*
 * JSON element containing the AWS secret access key.
 */
#define JSON_AWS_SECRET_ACCESS_KEY "SecretAccessKey"

/*
 * JSON element containing the token
 */
#define JSON_AWS_TOKEN "Token"

/*
 * The number of tokens (i.e. tokens in the context of the jsmn JSON
 * processor) that are in the EC2 metadata response.  (Note: there are
 * actually only 14 tokens, however, the squirrelly jsmn JSON processor
 * requires one extra to handle the trailing bracket.)
 */
#define JSON_AWS_TOKEN_COUNT 15

/*
 * New credentials should be made available when we are within 5 minutes
 * of expiration.  Set it a little less than 5 min to enusre that we're
 * within the window.
 */
#define CREDENTIAL_EXPIRATION_WINDOW (4 * 60)

/*
 * Prototype for function that will determine if we need to get new 
 * AWS credentials.
 */
int aws_credential_update_needed(gchar *aws_time);

/*
 * Prototype for function that will retrieve the AWS credential data
 * associated with the IAM role.
 */
int get_aws_credentials(aws_credentials *creds, const gchar *iam_role);

/* print the credentials as needed for debugging */
void print_aws_credentials(aws_credentials *creds);

#endif /* INCLUDE_EC2_METADATA_H_ */
