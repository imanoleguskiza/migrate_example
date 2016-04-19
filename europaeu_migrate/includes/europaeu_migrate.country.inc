<?php

/**
 * @file
 * Contains IntegrationTestArticlesMigration class.
 */

/**
 * Class EuropaEuCountryMigration .
 */
class EuropaEuCountryMigration extends EuropaEuAbstractMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct('country', $arguments);

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationNode::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationNode('country', array('text_format' => 'full_html')));

    // Entity translation requires that both title fields are mapped.
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('title_field', 'title');
    $this->addFieldMapping('body', 'body');
    $this->addFieldMapping('field_country_overview', 'overview');
    $this->addFieldMapping('field_country_image', 'map');
    $this->addFieldMapping('field_country_more', 'read_more_link');
    $this->addFieldMapping('field_country_more:title', 'read_more');

    $this->addFieldMapping('field_country_capital', 'country_capital');
    $this->addFieldMapping('field_country_size', 'country_size');
    $this->addFieldMapping('field_country_population', 'country_population');
    $this->addFieldMapping('field_country_population_as', 'country_population_as');
    $this->addFieldMapping('field_country_gdp', 'country_gdp');
    $this->addFieldMapping('field_country_languages', 'country_languages');
    $this->addFieldMapping('field_country_polsys', 'country_polsys');
    $this->addFieldMapping('field_country_membersince', 'country_membersince');
    $this->addFieldMapping('field_country_seats', 'country_seats');
    $this->addFieldMapping('field_country_currency', 'country_currency');
    $this->addFieldMapping('field_country_schengen', 'country_schengen');
    $this->addFieldMapping('field_country_presidency', 'country_presidency');
    $this->addFieldMapping('field_dynamic_submenu')->defaultValue(1);

    // Mapping default language is necessary for correct translation handling.
    $this->addFieldMapping('language', 'default_language');

    $this->addFieldMapping('promote')->defaultValue(FALSE);
    $this->addFieldMapping('status')->defaultValue(NODE_PUBLISHED);
  }

  /**
   * Implements prepareRow().
   */
  public function prepareRow($row) {
    $row->body = _europaeu_migrate_import_contents_filter($row->body, '');
    $domain = variable_get('europaeu_migrate_domain_name');
    $row->map = $domain . $row->map;

    // Extract from read_more.
    $dom = new \DOMDocument();
    libxml_use_internal_errors(TRUE);
    $html = (string) $row->read_more;
    $dom->loadHTML(mb_convert_encoding($html, 'HTML-ENTITIES', 'UTF-8'));

    $row->read_more = [];
    $row->read_more_link = [];
    foreach ($dom->getElementsByTagName('a') as $element) {
      if (!empty(trim($element->nodeValue))) {
        $row->read_more[] = $element->nodeValue;
        $row->read_more_link[] = $element->getAttribute('href');
      }
    }

    $xpath = new \DOMXpath($dom);
    foreach ($xpath->query('*//li') as $item) {
      $text = trim($item->textContent);
      if (!empty($text) && (count($item->getElementsByTagName('img')) > 0)) {
        if ($item->getElementsByTagName('img')->item(0) !== NULL) {
          $links = $item->getElementsByTagName('a');
          foreach ($links as $link) {
            $row->read_more[] = $text . ' ' . ucfirst($link->getElementsByTagName('img')->item(0)->getAttribute('alt'));
            $row->read_more_link[] = $link->getAttribute('href');
          }
        }
      }
    }

    $html = $row->info1 . $row->info2;
    $dom = new \DOMDocument();
    libxml_use_internal_errors(TRUE);
    $html = (string) $html;
    $dom->loadHTML(mb_convert_encoding($html, 'HTML-ENTITIES', 'UTF-8'));
    $xpath = new \DOMXpath($dom);

    $fields_settings = [
      'country_capital' => ["Capital", "Столица", "Hlavní město", "Hovedstad", "Hauptstadt:", "Pealinn", "Πρωτεύουσα", "Capital", "Capitale", "Príomhchathair:", "Glavni grad:", "Capitale:", "Galvaspilsēta:", "Sostinė:", "Főváros:", "Kapitali:", "Hoofdstad:", "Stolica:", "Capital:", "Capitala:", "Hlavné mesto:", "Glavno mesto:", "Pääkaupunki:", "Huvudstad:"],
      'country_size' => ["Geographical size", "Площ", "Rozloha", "Areal", "Fläche:", "Pindala", "Έκταση", "Superficie", "Superficie", "Achar na tíre:", "Površina:", "Superficie:", "Platība:", "Plotas:", "Terület:", "Daqs ġeografiku:", "Oppervlakte:", "Powierzchnia:", "Superfície:", "Suprafața:", "Zemepisná rozloha:", "Površina:", "Pinta-ala:", "Yta:"],
      'country_population' => ["Population", "Население", "Počet obyvatel", "Befolkningstal", "Einwohner:", "Rahvaarv", "Πληθυσμός", "Población", "Population", "Daonra:", "Broj stanovnika:", "Popolazione:", "Iedzīvotāju skaits:", "Gyventojų skaičius:", "Népesség:", "Popolazzjoni", "Bevolking:", "Liczba mieszkańców:", "População:", "Populația:", "Obyvateľstvo:", "Prebivalstvo:", "Väkiluku:", "Folkmängd:"],
      'country_population_as' => ["Population as % of total EU population", "Дял от общото население на ЕС", "Podíl na&nbsp;celkovém počtu obyvatel EU", "Befolkningstal som % af den samlede befolkning i EU", "Bevölkerung in % der EU-Gesamtbevölkerung:", "Rahvaarvu osakaal (%) kogu ELi rahvaarvust", "Πληθυσμός ως ποσοστό του συνολικού πληθυσμού της ΕΕ", "Porcentaje de la población total de la UE", "Population en % du total de l'UE", "Daonra mar % de dhaonra iomlán an AE:", "Broj stanovnika kao postotak ukupnog stanovništva EU-a:", "Popolazione in % della popolazione totale dell’UE:", "Iedzīvotāju īpatsvars % no kopējā ES iedzīvotāju skaita:", "Gyventojų dalis (%), palyginti su bendru ES gyventojų skaičiumi:", "A népességnek az EU összlakosságához viszonyított aránya:", "Popolazzjoni bħala % tal-popolazzjoni totali tal-UE:", "Bevolking in % van de totale EU-bevolking:", "Udział w liczbie ludności UE:", "População em % da população total da UE:", "Populația ca procent din totalul populației UE:", "Obyvateľstvo ako&nbsp;% z celkového počtu obyvateľov EÚ:", "Prebivalstvo v % skupnega prebivalstva EU:", "Osuus EU:n väkiluvusta:", "Folkmängd i procent av EU:s befolkning:"],
      'country_gdp' => ["GDP", "БВП", "HDP", "BNP", "BIP:", "SKP", "ΑΕΠ", "PIB", "PIB", "Olltáirgeacht intíre:", "BDP:", "PIL:", "IKP:", "BVP:", "Bruttó hazai termék (GDP):", "PDG:", "Bbp:", "PKB:", "PIB:", "PIB:", "HDP:", "BDP:", "BKT:", "BNP:"],
      'country_languages' => ["Official EU language(s)", "Официални езици на ЕС", "Úřední jazyky EU", "Officielle EU-sprog:", "EU-Amtssprache(n):", "ELi ametlikud keeled", "Επίσημη(-ες) γλώσσα(-ες) της ΕΕ", "Lenguas oficiales de la UE", "Langue(s) officielle(s) de l'UE", "Teangacha oifigiúla de chuid an AE:", "Službeni jezik EU-a:", "Lingue ufficiali dell’UE:", "ES oficiālā valoda:", "Oficialioji ES kalba:", "Hivatalos uniós nyelv(ek):", "Lingwa(i) uffiċjali tal-UE:", "Officiële EU-talen:", "Język urzędowy:", "Língua oficial da UE:", "Limba oficială în UE:", "Úradné jazyky EÚ:", "Uradni jezik EU:", "Virallinen EU-kieli:", "Officiella EU-språk:"],
      'country_polsys' => ["Political system", "Политическа система", "Politický systém", "Styreform:", "Staatsform:", "Poliitiline süsteem", "Πολίτευμα", "Sistema político", "Régime politique", "Córas polaitíochta:", "Politički sustav", "Sistema politico:", "Valsts iekārta", "Politinė sistema", "Államforma, politikai rendszer:", "Sistema politika:", "Staatsbestel:", "Ustrój polityczny:", "Sistema político:", "Sistem politic:", "Politický systém:", "Politični sistem", "Poliittinen järjestelmä", "Statsskick:"],
      'country_membersince' => ["EU member state since", "Член на ЕС от", "Vstup do EU", "EU-medlemsland siden", "EU-Mitgliedstaat seit:", "ELi liikmesriik alates", "Κράτος μέλος της ΕΕ από", "Miembro de la UE desde", "Date d'adhésion à l'UE", "Ina Ballstát ón:", "Datum pristupanja EU-u:", "Paese membro dell’UE dal:", "ES dalībvalsts no:", "ES valstybė narė nuo:", "Az Európai Unió tagországa:", "Pajjiż membru tal-UE:", "EU-lidstaat sinds:", "W UE od:", "Adesão à UE:", "Stat membru UE de la:", "Členstvo v EÚ od:", "Država članica EU od:", "Liittyi EU:n jäseneksi:", "EU-medlem sedan:"],
      'country_seats' => ["Seats in the European Parliament", "Места в Европейския парламент", "Počet křesel v&nbsp;Evropském parlamentu", "Pladser i Europa-Parlamentet", "Sitze im Europäischen Parlament:", "Kohti Euroopa Parlamendis", "Αριθμός εδρών στο Ευρωπαϊκό Κοινοβούλιο", "Escaños en el Parlamento Europeo", "Nombre de sièges au Parlement européen", "Líon a Feisirí i bParlaimint na hEorpa:", "Broj mjesta u Europskom parlamentu:", "Seggi al Parlamento europeo:", "Deputātu skaits Eiropas Parlamentā:", "Vietų skaičius Europos Parlamente:", "Képviselői helyek száma az Európai Parlamentben:", "Siġġijiet fil-Parlament Ewropew:", "Aantal zetels in het Europees Parlement:", "Liczba posłów w Parlamencie Europejskim:", "Lugares no Parlamento Europeu:", "Locuri în Parlamentul European:", "Počet poslancov v Európskom parlamente:", "Število sedežev v Evropskem parlamentu:", "Paikkoja Euroopan parlamentissa:", "Antal platser i Europaparlamentet:"],
      'country_currency' => ["Currency:", "Валута", "Měna:", "Valuta:", "Währung:", "Rahaühik:", "Νόμισμα", "Moneda:", "Monnaie:", "Airgeadra:", "Valuta:", "Valuta:", "Valūta:", "Valiuta:", "Pénznem:", "Munita:", "Munteenheid:", "Waluta:", "Moeda:", "Moneda:", "Mena:", "Valuta:", "Rahayksikkö:", "Valuta:"],
      'country_schengen' => ["Schengen area member?", "Член на Шенгенското пространство?", "Je součástí schengenského prostoru?", "Med i Schengenområdet?", "Mitglied des Schengenraums?", "Schengeni ala liige?", "Μέλος του Χώρου Σένγκεν;", "¿Pertenece al espacio Schengen?:", "Membre de", "Limistéar Schengen:", "Pripada li schengenskom prostoru?", "Fa parte dello spazio Schengen?", "Šengenas zonas valsts?", "Ar priklauso Šengeno erdvei?", "Schengen-tagság:", "Membru taż-żona Schengen?", "Schengenland?", "Czy należy do strefy Schengen?", "Membro do Espaço Schengen?", "Posibilitatea de a călători fără pașaport:", "Štát schengenského priestoru?", "Članica schengenskega območja?", "Schengen-jäsenyys:", "Schengenland?"],
      'country_presidency' => ["Presidency of the Council", "Председателство на Съвета", "Předsednictví v Radě", "Formandskab for Rådet", "Ratspräsidentschaft:", "Euroopa Liidu Nõukogu eesistuja", "Προεδρία του Συμβουλίου", "Presidencia del Consejo", "Présidence du Conseil", "Uachtaránacht na Comhairle:", "Predsjedanje Vijećem:", "Presidenza del Consiglio:", "Padomes prezidentūra:", "Pirmininkavimas Tarybai:", "Soros uniós elnökség:", "Presidenza tal-Kunsill:", "Voorzitterschap van de Raad:", "Przewodnictwo w Radzie:", "Presidência do Conselho:", "Președinția Consiliului:", "Predsedníctvo Rady:", "Predsedovanje Svetu EU:", "EU:n neuvoston puheenjohtajuus:", "Ordförandeland i EU:"],
    ];

    foreach ($xpath->query('*//p') as $paragraph_element) {
      $strong_element = $paragraph_element->getElementsByTagName('strong')->item(0);
      $field_label = trim($strong_element->nodeValue);
      $paragraph_element->removeChild($strong_element);
      foreach ($paragraph_element->childNodes as $child_node) {
        // Remove the ":" char from the value.
        if ($child_node->nodeName == '#text' && strstr($child_node->nodeValue, ':') !== FALSE) {
          $child_node->nodeValue = ltrim($child_node->nodeValue, ':');
        }
      }
      foreach ($fields_settings as $row_field_name => $field_labels) {
        if (in_array($field_label, $field_labels)) {
          $newdoc = new DOMDocument();
          $newdoc->appendChild($newdoc->importNode($paragraph_element, TRUE));
          $row->{$row_field_name} = $newdoc->saveHTML();
          unset($fields_settings[$row_field_name]);
        }
      }
    }

    return TRUE;
  }

  /**
   * Implements prepare().
   */
  public function prepare(&$entity, $row) {
    $alias = trim(_europaeu_migrate_path($row->original_url), '/');
    $entity->path['pathauto'] = 0;
    if (isset($entity->nid)) {
      $path = array(
        'source' => "node/$entity->nid",
        'alias' => $alias,
        'language' => $entity->language,
      );
      path_save($path);
      unset($entity->path['alias']);
    }
    else {
      $entity->path['alias'] = $alias;
    }
  }


  /**
   * Implements complete().
   */
  public function complete($entity, \stdClass $source_row) {
    parent::complete($entity, $source_row);
    $alias = drupal_get_path_alias('node/' . $entity->nid);
    if (!empty($alias)) {
      $menu_info = db_select('menu_links', 'ml')
        ->condition('ml.link_path', $alias)
        ->condition('ml.menu_name', 'main-menu')
        ->fields('ml', array('mlid'))
        ->execute()
        ->fetchAll();
      if (count($menu_info) > 0) {
        foreach ($menu_info as $key => $value) {
          $menu_link_item = menu_link_load($menu_info[$key]->mlid);
          $menu_link_item['link_path'] = 'node/' . $entity->nid;
          menu_link_save($menu_link_item);
        }
      }
    }
  }

}
