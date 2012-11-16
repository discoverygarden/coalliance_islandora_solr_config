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
class ADRCOSPLRSSResults extends IslandoraSolrResults {

  static $facetSeparator = '~'; //used to separate facets in url
  static $slashReplacement = '~slsh~'; // a pattern used to replace / in url's the slash breaks drupal clean url's
  public static $SEARCH_CLASS_ADVANCED_SEARCH_NUMBER_FIELDS = 5;

  function ADRCOSPLRSSResults() {
    parent::__construct();
  }



  function printRSS($solrQueryProcessor, $title = "Search Results") {
    global $base_url;
    module_load_include('inc', 'islandora_co_solr_config', 'ADRCOSPLSolrResults');
    $co_results = new ADRCOSPLSolrResults();    
    $upperLimit = $solrQueryProcessor->solrResult->response->numFound;
    $lowerLimit = 0;

    // set limit to rss items.
    $limit = variable_get('islandora_solr_search_num_of_results', 20);
    if (isset($_GET['limit'])) {
      $limit = $_GET['limit'];
    }
    if($limit == -1) {
      $limit = 30000;
    }
    $solrQueryProcessor->solrParams['hl'] = '';
    $solrQueryProcessor->solrParams['hl.fl'] = '';
    $solrQueryProcessor->solrStart = $lowerLimit;
    $solrQueryProcessor->solrLimit = $limit;
    $solrQueryProcessor->executeQuery();
    $rawResponse = $solrQueryProcessor->solrResult->getRawResponse();
    $responseData = json_decode($rawResponse, true);
    $docs = $responseData['response']['docs'];
    
    // this is where we switch to rss view
    drupal_set_header('Content-Type: application/rss+xml; charset=utf-8');
    

    // set variables
    $result = array();
    $item = array();
   
    $feed = '';
    if (isset($_GET['feed'])) {
      $feed = $_GET['feed'];
    }
    // render the single rss <item> elements
    foreach ($docs as $key => $doc) {
      if (empty($doc['Title'])) {
        continue;
      }
    $rss_source = variable_get('site_name', "Default site name");
    $rss_source_url = $base_url;
   
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

      $result['description'] = '<table>' . $co_results->build_list($doc, null, null, FALSE) .'</table>';


      $result['items'] = array(
        array('key' => 'guid', 'value' => $doc['PID'], 'attributes' => array('isPermaLink' => 'false',)),
        array('key' => 'pubDate', 'value' => $doc['fgs.createdDate']),
        array('key' => 'category', 'value' => $doc['Subjects']),
        //array('key' => 'enclosure', 'value' => '', 'attributes' => array('url' => '', 'length' => '', 'type' => '')),
        array('key' => 'comments', 'value' => ''),
        array('key' => 'source', 'value' => $rss_source, 'attributes' => array('url' => $rss_source_url)),
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


