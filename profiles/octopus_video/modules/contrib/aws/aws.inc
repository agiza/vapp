<?php

/**
 * @file
 *   An include file of an base module for for common functions that are used by
 *   the sub-modules enabling specific Amazon Web Services API for
 *   other Drupal modules.
 */

/* ******************** AWS Common Functions **********************************/

/* ******************** AWS REST API Functions ********************************/

/**
 * AWS REST request controller.
 * @public
 * @param array $request see README
 * @param array $headers see README
 * @param array $data    see README
 * @param int   $retry   number of retries if redirected
 * @return object std class see README
 */
function aws_rest_request($request = array(), $headers = array(), $data = NULL, $retry = 3) {
  $headers = aws_build_rest_header($request, $headers, $data);
  return rest_client_request($request, $headers, $data, $retry);
}

/**
 * Builds the needed headers for AWS over REST,  including signing the header.
 * @private
 * @param array $request see README
 * @param array $headers see README
 * @param array $data    see README
 * @return array of headers to be used in the HTTP request
 */
function aws_build_rest_header($request, $headers, $data) {


  // get AWS access key
  $access_key = cloud_get_info( $cloud_context, 'aws_access_key' )  ;

  // set request defaults
  $request['host'       ] = $headers['host'];
  $request['method'     ] = $request['method'     ] ? $request['method']      : 'GET';
  $request['resource'   ] = $request['resource'   ] ? $request['resource']    : '/'   ;
  $request['port'       ] = $request['port'       ] ? $request['port']        : '80'  ;
  $request['httpversion'] = $request['httpversion'] ? $request['httpversion'] : '1.0';

  // set defaults
  $headers['content-md5'] = $data ? base64_encode(rest_client_binary_md5($data)) : '';
  // not using DATE_RFC822 constant to be PHP4 compat, DATE_RFC822 > PHP 5.1.1
  $headers['date'] = gmdate('D, d M Y H:i:s T');

  if ($data && ! $headers['content-type']) {
    $headers['content-type'] = ($data['type'] === 'string') ? 'text/plain' : 'binary/octet-stream';
  }

  // set authorization with header signature
  $headers['authorization'] = 'AWS ' . $access_key . ':' . aws_header_sign($request, $headers);

  return $headers;
}

/**
 * Creates the header signature
 * @private
 * @param array $request see README
 * @param array $headers see README
 * @return string header signature
 */
function aws_header_sign($request, $headers) {
  $strtosign = drupal_strtoupper($request['method']) . "\n" . $headers['content-md5']
             . "\n" . $headers['content-type'] . "\n" . $headers['date'       ];

  // Format and sort x-amz headers
  $h = array();
  foreach ($headers as $key => $val) {
    $key = trim(drupal_strtolower($key));
    $val = trim($val);
    if (substr($key, 0, 5) === 'x-amz') {
      if (isset($h[$key])) {
        $h[$key] .= ", $val";
      }
      else {
        $h[$key] = "$key:$val";
      }
    }
  }

  if ($h !== array()) {
    ksort($h);
    $h = implode("\n", $h);
    $h = "\n$h";
  }
  else {
    $h = '';
  }

  // get AWS secret access key
  $secret_key = cloud_get_info( $cloud_context, 'aws_secret_key' )  ;

  // Build and sign the string
  $strtosign .=  $h . "\n" . $request['cresource'];
  return aws_hmac($strtosign, $secret_key);
}




/* ******************** AWS Query API Functions *******************************/

/**
 * AWS Query request controller.
 * @public
 * @param string $host request host
 * @param array $query key => value pair array holding query parameters and values
 * @param bool $ssl TRUE to use https
 * @param int $retry   number of retries if redirected
 * @return object std class see README
 */
function aws_query_request($cloud_context, $host, $request = array(), $ssl = FALSE, $headers = array(), $retry = 3) {

  $query  = aws_build_query_string($cloud_context, $request);
  $scheme = $ssl ? 'https' : 'http';
  $url    = $scheme . '://' . $host . '/?' . $query;

  return drupal_http_request($url, array('headers' => $headers, 'method' => 'GET', 'data' => NULL, 'max_redirects' => $retry, 'timeout' => 10000 ));
  // return sendRequest($url, $headers,  'GET',  NULL, $retry);
}


/**
 * Builds the querystring for AWS over QUERY,  including signing the header.
 * @private
 * @param array $query key => value pair array holding query parameters and values
 * @param array $headers see README
 * @param array $data    see README
 * @return the query string to be used in the HTTP request
 */
function aws_build_query_string($cloud_context, $query ) {

  // required querystring values
  $query['AWSAccessKeyId'  ] = cloud_get_info( $cloud_context, 'aws_access_key' )  ;
  $query['SignatureVersion'] = 1;
  $query['SignatureMethod'] = 'HmacSHA1'; // Needed for OpenStack

  // Use an expiry if we've set one.
  // Amazon defaults to 15 minutes when a timestamp is used instead
  //$query['Expires'] = !empty($query['Expires']) ? $query['Expires'] : '';
  if ( empty($query['Expires'])) {
    $query['Timestamp'] = gmdate('Y-m-d\TH:i:s') . 'Z';
  }

  // set authorization with header signature
  $query['Signature'] = aws_query_sign($cloud_context, $query);

  //return the encoded querystring
  return drupal_http_build_query($query);
}

/**
 * Creates the querystring signature
 * @private
 * @param array $request see README
 * @return string signature
 */
function aws_query_sign($cloud_context, $query) {

  // Amazon need items sorted case-insensitively
  uksort($query,    'strnatcasecmp');

  // get AWS secret access key
  $secret_key = cloud_get_info( $cloud_context, 'aws_secret_key' )  ;
  $strtosign = '';
  // Build and sign the string
  foreach ($query as $key => $value) {
    $strtosign .= $key . $value;
  }

  return aws_hmac($strtosign, $secret_key);
}

/**
 * Converts an array to array with numbered labels ('item.1' => 'data', 'item.2' => 'data')
 * @private
 * @param string $label  Label name to prepend .<itemcount>
 * @param array $items   Array of items
 * @return array formatted array
 */
function aws_number_keys($label, $items = array()) {

  $result = array();
  $itemcount = 1;
  foreach ($items as $item) {
    $result[$label . '.' . ++$itemcount] = $item;
  }

  return $result;
}



/* ***********************    Utility functions   *****************************/

/**
 * Creates a HMAC code
 * @public
 * @param string $data data to be encoded
 * @param sting $key key to use for encryption
 * @return string binary version of a sha1 hash
 */
function aws_hmac($data, $key) {

  if (strlen($key) < 64) {
    $key = str_pad($key,    64,    chr(0));
  }

  $ipad = (substr($key,    0,    64) ^ str_repeat(chr(0x36),    64));
  $opad = (substr($key,    0,    64) ^ str_repeat(chr(0x5c),    64));

  $hmac = aws_binary_sha1($opad . aws_binary_sha1($ipad . $data));

  return base64_encode($hmac);
}

/**
 * Creates a binary sha1 hash
 * @public
 * @param string $data
 * @return string binary version of a sha1 hash
 */
function aws_binary_sha1($data) {

  if (version_compare(PHP_VERSION, '5.0.0', '>=')) {
    return sha1($data, TRUE);
  }
  else {
    return pack('H40', sha1($data));
  }
}
