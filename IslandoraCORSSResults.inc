<?php

/**
 * @file
 * contains methods to search solr and display results.  depends on Apache_Solr_Php client.
 */
module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

/**
 * Extention of IslandoraSolrResults for templating purposes.
 * This overrides the displayResults function to provide an alternate display type.
 */
class IslandoraCORSSResults extends IslandoraSolrResults {

  static $facetSeparator = '~'; //used to separate facets in url
  static $slashReplacement = '~slsh~'; // a pattern used to replace / in url's the slash breaks drupal clean url's
  public static $SEARCH_CLASS_ADVANCED_SEARCH_NUMBER_FIELDS = 5;

  function IslandoraCORSSResults() {
    parent::__construct();
//module_load_include('php', 'islandora_solr_search', 'Solr/Service');
  }



  function printRSS($solrQueryProcessor, $title = "Search Results") {
    global $base_url;
    module_load_include('inc', 'islandora_co_solr_config', 'IslandoraCOSolrResults');
    $co_results = new IslandoraCOSolrResults();    
    $upperLimit = $solrQueryProcessor->solrResult->response->numFound;
    $lowerLimit = 0;
    $increment = 1000;



    $row_count = 0;
    $field_counts = array();
    $values = array();

    $solrQueryProcessor->solrStart = $lowerLimit;
    $solrQueryProcessor->solrLimit = $increment;

    while ($solrQueryProcessor->solrStart < 25) {
      $rawResponse = $solrQueryProcessor->solrResult->getRawResponse();
      $responseData = json_decode($rawResponse, true);
// Perform the incremental re-query.
      $solrQueryProcessor->resetResults();
      $solrQueryProcessor->executeQuery();

// Update incremental limits for the next round.
      $lowerLimit += $increment;
      $solrQueryProcessor->solrStart = $lowerLimit;

// If there are no results, skip ahead (i.e. exit).
      if (empty($solrQueryProcessor->solrResult)) {
        continue;
      }

// else ...
      $rawResponse = $solrQueryProcessor->solrResult->getRawResponse();
      $responseData = json_decode($rawResponse, true); // true == associative array, not object
//dsm($responsedata);
      unset($rawResponse); // unknown usefulness

      $docs = $responseData['response']['docs'];
//dsm($docs);
    }




// this is where we switch to rss view
    drupal_set_header('Content-Type: application/rss+xml; charset=utf-8');

// set variables
    $result = array();
    $item = array();

// set limit to rss items.
    $limit = variable_get('islandora_solr_custom_rss_limit', 25);
    if (isset($_GET['limit'])) {
      $limit = $_GET['limit'];
    }
    $docs = array_slice($docs, 0, $limit);
    $feed = '';
    if (isset($_GET['feed'])) {
      $feed = $_GET['feed'];
    }
// render the single rss <item> elements
    foreach ($docs as $key => $doc) {
      if (empty($doc['Title'])) {
        continue;
      }

// some values are located in a single array. Revert this:
      foreach ($doc as $key => $value) {

        if (is_array($value) AND count($value == 1)) {
          $doc[$key] = $value[0];
        }
      }

      $result = array();
      $result['title'] = $doc['Title'];
//      $result['link'] = $base_url . '/fedora/repository/' . htmlspecialchars($doc['PID'], ENT_QUOTES, 'utf-8');
      $result['link'] = $base_url . '/fedora/repository/' . $doc['PID'];

      $result['description'] = '<table>' . $co_results->build_list($doc, null, null) .'</table>';


      $result['items'] = array(
        array('key' => 'guid', 'value' => $doc['PID'], 'attributes' => array('isPermaLink' => 'false',)),
        array('key' => 'pubDate', 'value' => $doc['fgs.createdDate']),
        array('key' => 'category', 'value' => $doc['Subjects']),
        array('key' => 'enclosure', 'value' => '', 'attributes' => array('url' => '', 'length' => '', 'type' => '')),
        array('key' => 'comments', 'value' => ''),
        array('key' => 'source', 'value' => ''),
      );

      $item[] = $result;
    }

    $items = '';
    foreach ($item as $feed) {
      $items .= format_rss_item($feed['title'], $feed['link'], $feed['description'], $feed['items']);
    }

// query search terms:
    $query = $solrQueryProcessor->solrQuery;

// get the variables for the <channel> element
    $channel = prepare_rss_channel($query, $feed);

// give the results clean variable names.
    $title = $channel['title'];
    $url = $channel['url'];
    $description = $channel['description'];
    $langcode = $channel['langcode'];
    $args = $channel['args'];

    $output = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    $output .= "<rss version=\"2.0\">\n";
    $output .= format_rss_channel($title, $url, $description, $items, $langcode, $args);
    $output .= "</rss>\n";
    print $output;
  }

}

function prepare_rss_channel($query, $feed) {
  global $base_url;
  $result['title'] = t('@site_name aggregator', array('@site_name' => variable_get('site_name', 'Drupal')));
  $result['url'] = $base_url;
  $result['description'] = t('Aggregated search results of:') . ' ' . $query;
  $result['langcode'] = NULL;
  $result['args'] = array();

return $result;
}


