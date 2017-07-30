int read_page(struct block_device *device, sector_t sector, int size, struct page *page)
{
	int ret;
	struct completion event;

	//allocate a new bio, memory poll backed
	// bio(gfp_mask, nr_iovecs): the last one is number of iovecs.
	struct bio *bio = bio_alloc(GFP_NOIO, 1);
	bio->bi_bdev = device;
	bio->bi_sector = sector;
	//attempt to add page to bio_vec maplist
	//bio_add_page(bio, page, len, offset):
	bio_add_page(bio, page, size, 0);

	init_completion(&event);
	bio->bi_private = &event;
	bio->bio_end_io = read_complete;
	// submit a bio to the block device layer for I/O
	submit_bio(READ|REQ_SYNC, bio);
	wait_for_completioni(&event);

	//release a reference to a bio
	ret = test_bit(BIO_UPTODATE, &bio->bio_flags);
	bio_put(bio);
	
	return ret;
}

void write_page(struct block_device *device, sector_t sector, int size, struct page *page)
{
	struct bio *bio = bio_alloc(GFP_NOIO, 1);
	bio->bi_bdev = vnode->blkDevice;
	bio->bi_sector = sector;
	bio_add_page(bio, page, size, 0);

	bio->bi_end_io = write_complete;
	submit_bio(WRITE_FLUSH_FUA, bio);
}

