<?php

/**
 * @file
 * Contains IntegrationTestArticlesMigration class.
 */

/**
 * Class EuropaEuCommonPageMigration .
 */
class EuropaEuDownloadsMigration extends EuropaEuAbstractMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct('downloads', $arguments);

    $this->setMap(new MigrateSQLMap(
      $this->getMachineName(),
      $this->getSourceKey(),
      MigrateDestinationNode::getKeySchema()
    ));
    $this->setDestination(new MigrateDestinationNode('downloads', array('text_format' => 'full_html')));

    // Entity translation requires that both title fields are mapped.
    $this->addFieldMapping('title', 'document_title');
    $this->addFieldMapping('title_field', 'document_title');
    $this->addFieldMapping('field_downloads_link', 'link');
    $this->addFieldMapping('field_downloads_link:title', 'title');

    // Mapping default language is necessary for correct translation handling.
    $this->addFieldMapping('language', 'default_language');

    $this->addFieldMapping('promote')->defaultValue(FALSE);
    $this->addFieldMapping('status')->defaultValue(NODE_PUBLISHED);
  }

}
